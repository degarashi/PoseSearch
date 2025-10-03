#include "my_db.hpp"
#include <QMessageBox>
#include "aux_f_q/q_value.hpp"
#include "aux_f_q/sql/exception.hpp"
#include "aux_f_q/sql/query.hpp"
#include "condition/condition.hpp"

namespace {
	const auto BLACKLIST_FILE = QStringLiteral("blacklist.sqlite3");
	const auto BLACKLIST_DB = QStringLiteral("blacklist");
	const auto BLACKLIST_TABLE = dg::sql::Name(BLACKLIST_DB, "Blacklist");
	// clang-format off
	// ブラックリスト用テーブルレイアウト
	const auto blacklist_layout = QStringLiteral(
		R"(
			CREATE TABLE IF NOT EXISTS %1 (
				hash	BLOB NOT NULL UNIQUE,
				CHECK(LENGTH(hash) == 64)
			)
		)").arg(BLACKLIST_TABLE.text());
	// clang-format on

	// スコア計算用の一時テーブルのレイアウト
	const auto score_layout = QStringLiteral("poseId INTEGER NOT NULL,"
											 "cond_index INTEGER NOT NULL,"
											 "score REAL NOT NULL,"
											 "PRIMARY KEY(poseId, cond_index)");
	// スコア計算用の一時テーブルの名前
	const dg::sql::Name ScoreTable{"temp", "score_accum"};
} // namespace
namespace dg {
	void LoadVecExtension(dg::sql::Database &db) {
		try {
			// sqlite-vec拡張機能をロード（環境依存のため例外処理追加）
			db.loadExtension("sqlite-vec.dll", "sqlite3_vec_init");
		}
		catch (const sql::CantLoadExtension &e) {
			qWarning() << "Failed to load sqlite-vec extension:" << e.what();
			throw;
		}

		// SQLiteのバージョンとsqlite-vecのバージョンを取得
		auto q = db.exec("SELECT sqlite_version(), vec_version();");
		if (q.next()) {
			// SQLiteのバージョン
			qDebug() << "SQLite Version: " << q.value(0).toString();
			// sqlite-vecのバージョン
			qDebug() << "sqlite-vec Version: " << q.value(1).toString();
		}
		else
			qWarning() << "Failed to retrieve SQLite/vec version";
	}
} // namespace dg

MyDatabase::MyDatabase(std::unique_ptr<dg::sql::Database> db) : _db(std::move(db)), _debugMode(false) {
	try {
		// タグリストを取得してメンバ変数に格納
		auto q = _db->exec("SELECT name FROM TagInfo");
		while (q.next()) {
			_tags.append(q.value("name").toString());
		}
		_db->attach(BLACKLIST_FILE, BLACKLIST_DB);
		// Blacklistテーブルを未作成の場合は定義
		_db->exec(blacklist_layout);
	}
	catch (const std::exception &e) {
		qWarning() << "Database initialization failed:" << e.what();
	}
}

const QStringList &MyDatabase::getTagList() const {
	return _tags;
}

dg::sql::Database &MyDatabase::database() const {
	return *_db;
}
QString MyDatabase::getTag(const int idx) const {
	if (idx < 0 || idx >= _tags.size()) {
		qWarning() << "Invalid tag index:" << idx;
		return {};
	}
	return _tags.at(idx);
}

void MyDatabase::addBlacklist(const FileId fileId) const {
	const auto hash = getFileHash(fileId);
	if (hash.isEmpty()) {
		qWarning() << "addBlacklist: empty hash for fileId" << EnumToInt(fileId);
		return;
	}
	_db->exec(QString("INSERT OR IGNORE INTO %1 (hash) VALUES (?)").arg(BLACKLIST_TABLE.text()), hash);
}
void MyDatabase::removeBlacklist(const FileId fileId) const {
	const auto hash = getFileHash(fileId);
	if (hash.isEmpty()) {
		qWarning() << "removeBlacklist: empty hash for fileId" << EnumToInt(fileId);
		return;
	}
	_db->exec(QString("DELETE FROM %1 WHERE hash = ?").arg(BLACKLIST_TABLE.text()), hash);
}
bool MyDatabase::isBlacklisted(const FileId fileId) const {
	const auto hash = getFileHash(fileId);
	if (hash.isEmpty()) {
		return false;
	}
	auto q = _db->exec(QString("SELECT 1 FROM %1 WHERE hash = ?").arg(BLACKLIST_TABLE.text()), hash);
	return q.next();
}
void MyDatabase::deleteBlacklist() {
	_db->exec("DELETE FROM blacklist.Blacklist");
	QMessageBox::information(nullptr, "Blacklist Cleared", "Done.");
}
namespace {
	// 検索時の最大件数
	constexpr int SearchAllLimit = 4096;
	// 一時テーブルの名前は他と被らなければ特になんでもいい
	const QString ResultTableName("result");

	// クエリ結果を std::vector<T> として取得するヘルパー関数
	template <typename T>
	std::vector<T> fetchAll(QSqlQuery &query, const int column = 0) {
		std::vector<T> result;
		while (query.next()) {
			if (!query.value(column).isValid()) {
				qWarning() << "Invalid value in fetchAll at column" << column;
				continue;
			}
			result.emplace_back(query.value(column).value<T>());
		}
		return result;
	}

	// QVariant から QVector3D を生成するヘルパー関数
	QVector3D ConvertVec3(const QVariant &vx, const QVariant &vy, const QVariant &vz) {
		return {
			dg::ConvertQV<float>(vx),
			dg::ConvertQV<float>(vy),
			dg::ConvertQV<float>(vz),
		};
	}
	// 指定されたテーブルから poseId に対応する QVector3D を1つ取得する関数
	std::optional<QVector3D> fetchSingleVec3(const dg::sql::Database &db, const QString &table, const PoseId poseId) {
		auto q = db.exec(QString("SELECT x, y, z FROM %1 WHERE poseId = ? LIMIT 1").arg(table), poseId);
		if (!q.next())
			return std::nullopt;
		return ConvertVec3(q.value(0), q.value(1), q.value(2));
	}

	// 指定された poseId の左右の方向ベクトルを取得する共通関数
	std::array<std::optional<QVector3D>, 2> fetchLimbDirs(const dg::sql::Database &db, const QString &table,
														  const PoseId poseId) {
		std::array<std::optional<QVector3D>, 2> dirs; // [0]=left, [1]=right
		auto q = db.exec(QString("SELECT is_right, x, y, z FROM %1 WHERE poseId = ?").arg(table), poseId);
		while (q.next()) {
			const int isRight = dg::ConvertQV<int>(q.value(0));
			const int idx = (isRight != 0) ? 1 : 0;
			dirs[idx] = ConvertVec3(q.value(1), q.value(2), q.value(3));
		}
		return dirs;
	}
} // namespace

PoseIds MyDatabase::query(const int limit, const std::vector<Condition *> &clist) const {
	if (clist.empty()) {
		qWarning() << "query called with empty condition list";
		return {};
	}

	// --- スコア計算用テーブル ---
	try {
		_db->dropTable(ScoreTable, true);
		_db->createTempTable(ScoreTable.table, score_layout, false);
	}
	catch (const std::exception &e) {
		qWarning() << "Failed to create score table:" << e.what();
		return {};
	}

	for (int index = 0; auto &&cond : clist) {
		try {
			const auto qp = cond->getSqlQuery({
				.outputTableName = ResultTableName,
				.ratio = cond->getRatio(),
			});
			qp.exec(*_db,
					QString("INSERT INTO %1 "
							"SELECT poseId, %2, score * :ratio FROM %3")
						.arg(ScoreTable.text())
						.arg(index)
						.arg(ResultTableName),
					SearchAllLimit);
		}
		catch (const std::exception &e) {
			qWarning() << "Condition query failed:" << e.what();
		}
		++index;
	}
	// scoreTableにずらっとスコアが入っているので
	// FilePathと関連付けてソートし取り出す
	auto q = _db->exec(QString("SELECT Pose.id, SUM(Result.score) AS score "
							   "	FROM %1 AS Result "
							   "INNER JOIN Pose "
							   "	ON Result.poseId = Pose.id "
							   "INNER JOIN File "
							   "	ON Pose.fileId = File.id "
							   // -- Blacklist除外 --
							   "LEFT OUTER JOIN %2 BL"
							   "  ON File.hash = BL.hash "
							   "WHERE BL.hash IS NULL "
							   // -------------------
							   "GROUP BY Result.poseId "
							   "ORDER BY score DESC "
							   "LIMIT ?")
						   .arg(ScoreTable.text())
						   .arg(BLACKLIST_TABLE.text()),
					   limit);
	// 結果の集計
	PoseIds res;
	while (q.next()) {
		if (!q.value(0).isValid()) {
			qWarning() << "Invalid poseId in query result";
			continue;
		}
		res.emplace_back(dg::ConvertQV<PoseId>(q.value(0)));
	}
	return res;
}

MyDatabase::QueryScore MyDatabase::getScore(const PoseId poseId) const {
	const auto QStr = QStringLiteral(R"(
		SELECT score, SUM(score) OVER() AS accum_score
			FROM %1
		WHERE poseId = ?
		ORDER BY cond_index ASC
	)");
	auto q = _db->exec(QString(QStr).arg(ScoreTable.text()), poseId);

	QueryScore ret;
	if (!q.next())
		throw dg::RuntimeError("Pose ID " + std::to_string(EnumToInt(poseId)) + " not found in score table.");

	ret.score = dg::ConvertQV<float>(q.value(1));
	do {
		const float score = dg::ConvertQV<float>(q.value(0));
		ret.individual.emplace_back(score);
	}
	while (q.next());

	return ret;
}

QString MyDatabase::getFilePath(const FileId fileId) const {
	auto q = _db->exec("SELECT File.path FROM File WHERE id=?", fileId);
	if (q.next())
		return q.value(0).toString();
	qWarning() << "File path not found for id" << EnumToInt(fileId);
	return {};
}
QByteArray MyDatabase::getFileHash(const FileId fileId) const {
	auto q = _db->exec("SELECT File.hash FROM File WHERE id=?", fileId);
	if (q.next())
		return dg::ConvertQV<QByteArray>(q.value(0));
	qWarning() << "File hash not found for id" << EnumToInt(fileId);
	return {};
}
FileId MyDatabase::getFileId(const PoseId poseId) const {
	auto q = _db->exec("SELECT fileId FROM Pose WHERE id=?", poseId);
	if (q.next())
		return dg::ConvertQV<FileId>(q.value(0));
	qWarning() << "FileId not found for poseId" << EnumToInt(poseId);
	return FileId{-1};
}
QRectF MyDatabase::getPoseRect(const PoseId poseId) const {
	auto q = _db->exec("SELECT x0, x1, y0, y1 FROM PoseRect WHERE poseId=?", poseId);
	if (q.next()) {
		const float x0 = dg::ConvertQV<float>(q.value(0));
		const float x1 = dg::ConvertQV<float>(q.value(1));
		const float y0 = dg::ConvertQV<float>(q.value(2));
		const float y1 = dg::ConvertQV<float>(q.value(3));
		return QRectF(QPointF{x0, y0}, QPointF{x1, y1});
	}
	qWarning() << "PoseRect not found for poseId" << EnumToInt(poseId);
	return {};
}

PoseInfo MyDatabase::getPoseInfo(const PoseId poseId) const {
	// torsoDir
	const auto torsoOpt = fetchSingleVec3(*_db, "MasseTorsoDir", poseId);
	if (!torsoOpt)
		throw dg::RuntimeError("MasseTorsoDir not found for poseId=" + std::to_string(EnumToInt(poseId)));
	const QVector3D torsoDir = *torsoOpt;

	// torsoDir(Method)
	QString method("unknown");
	{
		auto q = _db->exec("SELECT method FROM MasseTorsoDir WHERE poseId = ?", poseId);
		if (q.next())
			method = dg::ConvertQV<QString>(q.value(0));
	}

	// thighDir (left/right)
	const auto thighDirs = fetchLimbDirs(*_db, "MasseThighDir", poseId);
	if (!thighDirs[0] || !thighDirs[1])
		throw dg::RuntimeError("MasseThighDir incomplete for poseId=" + std::to_string(EnumToInt(poseId)));

	// crusDir (left/right)
	const auto crusDirs = fetchLimbDirs(*_db, "MasseCrusDir", poseId);
	if (!crusDirs[0] || !crusDirs[1])
		throw dg::RuntimeError("MasseCrusDir incomplete for poseId=" + std::to_string(EnumToInt(poseId)));

	std::vector<QVector2D> landmarks;
	{
		// clang-format off
		auto q = _db->exec(R"(
			SELECT td_x, td_y, landmarkIndex
				FROM Landmark
				WHERE poseId = ?
				ORDER BY landmarkIndex ASC
		)", poseId);
		// clang-format on
		while (q.next()) {
			if (!q.value(0).isValid() || !q.value(1).isValid()) {
				qWarning() << "Invalid landmark value for poseId" << EnumToInt(poseId);
				continue;
			}
			landmarks.emplace_back(dg::ConvertQV<float>(q.value(0)), dg::ConvertQV<float>(q.value(1)));
		}
	}
	std::array<dg::Radian, 2> thighFlexInfo;
	{
		// clang-format off
		auto qThighFlex = _db->exec(R"(
			SELECT is_right, angleRad
				FROM ThighFlexion
				WHERE poseId = ?
				ORDER BY is_right ASC
			)", poseId);
		// clang-format on
		while (qThighFlex.next()) {
			const int isRight = dg::ConvertQV<int>(qThighFlex.value(0));
			const int idx = (isRight != 0) ? 1 : 0;
			thighFlexInfo[idx].set(dg::ConvertQV<float>(qThighFlex.value(1)));
		}
	}
	std::array<dg::Radian, 2> crusFlexInfo;
	{
		// clang-format off
		auto qCrusFlex = _db->exec(R"(
			SELECT is_right, angleRad
				FROM CrusFlexion
				WHERE poseId = ?
				ORDER BY is_right
			)", poseId);
		// clang-format on
		while (qCrusFlex.next()) {
			const int isRight = dg::ConvertQV<int>(qCrusFlex.value(0));
			const int idx = (isRight != 0) ? 1 : 0;
			crusFlexInfo[idx].set(dg::ConvertQV<float>(qCrusFlex.value(1)));
		}
	}

	return PoseInfo{std::move(landmarks),
					method,
					torsoDir,
					{*(thighDirs[0]), *(thighDirs[1])},
					{*(crusDirs[0]), *(crusDirs[1])},
					{thighFlexInfo[0], thighFlexInfo[1]},
					{crusFlexInfo[0], crusFlexInfo[1]},
					getPoseRect(poseId)};
}
