#include "my_db.hpp"
#include "aux_f/sql/exception.hpp"
#include "aux_f/sql/query.hpp"
#include "condition/condition.hpp"

namespace {
	const auto score_layout = QStringLiteral("poseId INTEGER NOT NULL,"
											 "score REAL NOT NULL");
}
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

MyDatabase::MyDatabase(std::unique_ptr<dg::sql::Database> db) : _db(std::move(db)) {
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
	constexpr int SearchAllLimit = 4096;
	// 一時テーブルの名前は他と被らなければ特になんでもいい
	const QString ResultTableName("result");
} // namespace

std::vector<int> MyDatabase::query(const int limit, const std::vector<Condition *> &clist) const {
	if (clist.empty())
		return {};

	const dg::sql::Name scoreTable{"temp", "score_accum"};
	// --- スコア計算用テーブル ---
	_db->dropTable(scoreTable, true);
	_db->createTempTable(scoreTable.table, score_layout, false);

	for (auto &&cond : clist) {
		const auto qp = cond->getSqlQuery({
			.outputTableName = ResultTableName,
			.ratio = cond->getRatio(),
		});
		// 単にResultテーブルへ追加
		qp.exec(*_db,
				QString("INSERT INTO %1 "
						"SELECT poseId, score FROM %2")
					.arg(scoreTable.text(), ResultTableName),
				SearchAllLimit);
	}
	// 結果の集計
	std::vector<int> res;
	// scoreTableにずらっとスコアが入っているので
	// FilePathと関連付けてソートし取り出す
	auto q = _db->exec(QString("SELECT File.id, SUM(Result.score) AS score "
							   "	FROM %1 AS Result "
							   "INNER JOIN Pose "
							   "	ON Result.poseId = Pose.id "
							   "INNER JOIN File "
							   "	ON Pose.fileId = File.id "
							   "GROUP BY Result.poseId "
							   "ORDER BY score DESC "
							   "LIMIT ?")
						   .arg(scoreTable.text()),
					   limit);
	while (q.next()) {
		const int id = q.value("id").toInt();
		qDebug() << id << ", " << q.value("score");
		res.push_back(id);
	}
	_db->dropTable(scoreTable);
	return res;
}

QString MyDatabase::getFilePath(const int fileId) const {
	auto q = _db->exec("SELECT File.path FROM File WHERE id=?", fileId);
	if (q.next())
		return q.value(0).toString();
	return {};
}
