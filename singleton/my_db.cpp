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
