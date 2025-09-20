#include "my_db.hpp"
#include "aux_f/q_value.hpp"
#include "aux_f/sql/exception.hpp"
#include "aux_f/sql/query.hpp"
#include "condition/condition.hpp"

namespace {
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
		// sqlite-vec.dll 拡張機能をロード
		// sqlite3_vec_init は拡張機能内の初期化関数
		db.loadExtension("sqlite-vec.dll", "sqlite3_vec_init");

		// SQLiteのバージョンとsqlite-vecのバージョンを取得
		auto q = db.exec("SELECT sqlite_version(), vec_version();");
		if (q.next()) {
			// SQLiteのバージョン
			qDebug() << "SQLite Version: " << q.value(0).toString();
			// sqlite-vecのバージョン
			qDebug() << "sqlite-vec Version: " << q.value(1).toString();
		}
	}
} // namespace dg

MyDatabase::MyDatabase(std::unique_ptr<dg::sql::Database> db) : _db(std::move(db)), _debugMode(false) {
	// タグリストを取得してメンバ変数に格納
	auto q = _db->exec("SELECT name FROM TagInfo");
	while (q.next()) {
		_tags.append(q.value("name").toString());
	}
}

const QStringList &MyDatabase::getTagList() const {
	return _tags;
}

dg::sql::Database &MyDatabase::database() const {
	return *_db;
}
QString MyDatabase::getTag(const int idx) const {
	return _tags.at(idx);
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
	std::optional<QVector3D> fetchSingleVec3(const dg::sql::Database &db, const QString &table, const int poseId) {
		auto q = db.exec(QString("SELECT x, y, z FROM %1 WHERE poseId = ? LIMIT 1").arg(table), poseId);
		if (!q.next())
			return std::nullopt;
		return ConvertVec3(q.value(0), q.value(1), q.value(2));
	}

	// 指定された poseId の左右の方向ベクトルを取得する共通関数
	std::array<std::optional<QVector3D>, 2> fetchLimbDirs(const dg::sql::Database &db, const QString &table,
														  const int poseId) {
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

std::vector<int> MyDatabase::query(const int limit, const std::vector<Condition *> &clist) const {
	if (clist.empty())
		return {};

	// --- スコア計算用テーブル ---
	_db->dropTable(ScoreTable, true);
	_db->createTempTable(ScoreTable.table, score_layout, false);

	for (int index = 0; auto &&cond : clist) {
		const auto qp = cond->getSqlQuery({
			.outputTableName = ResultTableName,
			.ratio = cond->getRatio(),
		});
		// 単にResultテーブルへ追加
		qp.exec(*_db,
				QString("INSERT INTO %1 "
						"SELECT poseId, %2, score * :ratio FROM %3")
					.arg(ScoreTable.text())
					.arg(index)
					.arg(ResultTableName),
				SearchAllLimit);
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
							   "GROUP BY Result.poseId "
							   "ORDER BY score DESC "
							   "LIMIT ?")
						   .arg(ScoreTable.text()),
					   limit);
	// 結果の集計
	std::vector<int> res;

	while (q.next()) {
		bool ok;
		const int poseId = q.value(0).toInt(&ok);
		Q_ASSERT(ok);
		res.emplace_back(poseId);
	}
	return res;
}

MyDatabase::QueryScore MyDatabase::getScore(const int poseId) const {
	const auto QStr = QStringLiteral(R"(
		SELECT score, SUM(score) OVER() AS accum_score
			FROM %1
		WHERE poseId = ?
		ORDER BY cond_index ASC
	)");
	auto q = _db->exec(QString(QStr).arg(ScoreTable.text()), poseId);

	QueryScore ret;
	if (!q.next())
		throw dg::RuntimeError(QString("Pose ID %1 not found in score table.").arg(poseId));

	bool ok;
	ret.score = q.value(1).toFloat(&ok);
	Q_ASSERT(ok);
	do {
		const float score = q.value(0).toFloat(&ok);
		Q_ASSERT(ok);
		ret.individual.emplace_back(score);
	}
	while (q.next());

	return ret;
}

QString MyDatabase::getFilePath(const int fileId) const {
	auto q = _db->exec("SELECT File.path FROM File WHERE id=?", fileId);
	if (q.next())
		return q.value(0).toString();
	return {};
}
int MyDatabase::getFileId(const int poseId) const {
	auto q = _db->exec("SELECT fileId FROM Pose WHERE id=?", poseId);
	if (q.next())
		return dg::ConvertQV<int>(q.value(0));
	return -1;
}

MyDatabase::PoseInfo MyDatabase::getPoseInfo(const int poseId) const {
	// torsoDir
	const auto torsoOpt = fetchSingleVec3(*_db, "MasseTorsoDir", poseId);
	if (!torsoOpt)
		throw dg::RuntimeError(QString("MasseTorsoDir not found for poseId=%1").arg(poseId));
	const QVector3D torsoDir = *torsoOpt;

	// thighDir (left/right)
	const auto thighDirs = fetchLimbDirs(*_db, "MasseThighDir", poseId);
	if (!thighDirs[0])
		throw dg::RuntimeError(QString("Left MasseThighDir not found for poseId=%1").arg(poseId));
	if (!thighDirs[1])
		throw dg::RuntimeError(QString("Right MasseThighDir not found for poseId=%1").arg(poseId));

	// crusDir (left/right)
	const auto crusDirs = fetchLimbDirs(*_db, "MasseCrusDir", poseId);
	if (!crusDirs[0])
		throw dg::RuntimeError(QString("Left MasseCrusDir not found for poseId=%1").arg(poseId));
	if (!crusDirs[1])
		throw dg::RuntimeError(QString("Right MasseCrusDir not found for poseId=%1").arg(poseId));

	std::vector<QVector2D> landmarks;
	{
		auto q = _db->exec("SELECT td_x, td_y FROM Landmark WHERE poseId = ?", poseId);
		while (q.next())
			landmarks.emplace_back(dg::ConvertQV<float>(q.value(0)), dg::ConvertQV<float>(q.value(1)));
	}

	return PoseInfo{
		std::move(landmarks),
		torsoDir,
		{*(thighDirs[0]), *(thighDirs[1])},
		{*(crusDirs[0]), *(crusDirs[1])},
	};
}
