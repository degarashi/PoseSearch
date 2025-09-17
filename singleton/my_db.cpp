#include "my_db.hpp"
#include "aux_f/sql/exception.hpp"
#include "aux_f/sql/query.hpp"
#include "condition/condition.hpp"

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
	void AddScoreAndDelTable1(dg::sql::Database &db, const dg::sql::Name &data0, const dg::sql::Name &data1) {
		// data0 + data1 -> tmp_table
		const QString tmpTable = QStringLiteral("calc_temp");
		const dg::sql::Name tmpTableN{"temp", tmpTable};
		db.createTempTable(tmpTable);
		db.exec(QString("INSERT INTO %1 (poseId, score) "
						"SELECT poseId, SUM(score) AS score "
						"FROM ( "
						"	SELECT poseId, score FROM %2 "
						"	UNION ALL "
						"	SELECT poseId, score FROM %3 "
						") "
						"GROUP BY poseId ")
					.arg(tmpTableN.text(), data0.text(), data1.text()));
		// 不要なテーブルを片付け
		db.dropTable(data0);
		db.dropTable(data1);
		// tmp_tableをdata0にリネーム
		db.renameTable(tmpTableN, data0.table);
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
std::vector<int> MyDatabase::query(const int limit, const std::vector<Condition *> &clist) const {
	if (clist.empty())
		return {};

	const dg::sql::Name temp0{"temp", "temp0"};
	const dg::sql::Name temp1{"temp", "temp1"};

	_db->dropTable(temp0, true);
	// 固定レイアウトのスコア計算用テーブル
	// CREATE TEMPORARY TABLE %2 %1 (
	//		poseId INTEGER NOT NULL,
	// 		score REAL NOT NULL
	// )
	_db->createTempTable(temp0.table);

	for (auto &&cond : clist) {
		// (この時点で消えている筈だが一応)
		_db->dropTable(temp1, true);
		_db->createTempTable(temp1.table);

		// 名前は他と被らなければ特になんでもいい
		const QString ResultTableName("result");
		// 足し合わせるテーブル(temp1)を作る為のクエリ
		const auto qp = cond->getSqlQuery({
			.outputTableName = ResultTableName,
			.limit = limit,
			.ratio = cond->getRatio(),
		});
		{
			QSqlQuery q(_db->database());
			q.prepare(qp.queryText +
					  QString("INSERT INTO %1 SELECT poseId, score*:ratio AS score FROM %2")
						  .arg(temp1.text(), ResultTableName));
			for (auto &p : qp.queryParams)
				q.bindValue(p.first, p.second);
			q.bindValue(":ratio", qp.ratio);
			dg::sql::Query(q);
		}
		// スコアの足し合わせ
		// [temp0 += temp1; drop(temp1)]
		dg::AddScoreAndDelTable1(*_db, temp0, temp1);
	}
	{
		// 単にデバッグ用
		auto q = _db->exec(QString("SELECT * "
								   "FROM %1 "
								   "ORDER BY score ASC")
							   .arg(temp0.text()));
		while (q.next()) {
			qDebug() << q.value(0) << ", " << q.value(1);
		}
	}
	// 結果の集計
	std::vector<int> res;
	{
		// temp.temp0に{poseId, score}の結果が入っているので、
		// FilePathと関連付けてソートし取り出す
		auto q = _db->exec(QString("SELECT File.id, Result.score "
								   "FROM %1 AS Result "
								   "INNER JOIN Pose "
								   "	ON Result.poseId = Pose.id "
								   "INNER JOIN File "
								   "	ON Pose.fileId = File.id "
								   "ORDER BY score DESC "
								   "LIMIT ?")
							   .arg(temp0.text()),
						   limit);
		while (q.next()) {
			const int id = q.value("id").toInt();
			qDebug() << id << ", " << q.value("score");
			res.push_back(id);
		}
	}
	_db->dropTable(temp0);
	return res;
}

QString MyDatabase::getFilePath(const int fileId) const {
	auto q = _db->exec("SELECT File.path FROM File WHERE id=?", fileId);
	if (q.next())
		return q.value(0).toString();
	return {};
}
