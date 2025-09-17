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
	void AddScore(dg::sql::Database &db, const dg::sql::Name &data0, const dg::sql::Name &data1) {
		// data0 + data1 -> tmp_table
		const QString tmpTable = QStringLiteral("calc_temp");
		const dg::sql::Name tmpTableN{"temp", tmpTable};
		db.createTempTable(tmpTable);
		db.exec(QString("INSERT INTO %1 (poseId, score)"
						"SELECT t0.poseId, t0.score + COALESCE(t1.score, 0) AS score "
						"FROM %2 AS t0 "
						"LEFT JOIN %3 AS t1 "
						"ON t0.poseId = t1.poseId "
						"UNION "
						"SELECT t1.poseId, t1.score + COALESCE(t0.score, 0) AS score "
						"FROM %3 AS t1 "
						"LEFT JOIN %2 AS t0 "
						"ON t1.poseId = t0.poseId ")
					.arg(tmpTableN.text(), data0.text(), data1.text()));
		db.dropTable(data0);
		db.dropTable(data1);
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
	_db->createTempTable(temp0.table);

	for (auto &&cond : clist) {
		_db->dropTable(temp1, true);
		_db->createTempTable(temp1.table);
		const auto qp = cond->getSqlQuery({
			.outputTableName = "result",
			.limit = limit,
			.ratio = cond->getRatio(),
		});
		{
			QSqlQuery q(_db->database());
			q.prepare(qp.queryText +
					  QString("INSERT INTO %1 SELECT poseId, score*:ratio AS score FROM result").arg(temp1.text()));
			for (auto &p : qp.queryParams)
				q.bindValue(p.first, p.second);
			q.bindValue(":ratio", qp.ratio);
			dg::sql::Query(q);
		}
		dg::AddScore(*_db, temp0, temp1);
	}

	auto q2 = _db->exec(QString("SELECT * "
								"FROM %1 "
								"ORDER BY score ASC")
							.arg(temp0.text()));
	while (q2.next()) {
		qDebug() << q2.value(0) << ", " << q2.value(1);
	}
	// temp.temp0に{poseId, score}の結果が入っているので、
	// FilePathと関連付けてソートし取り出す
	auto q = _db->exec(QString("SELECT File.id, score "
							   "FROM %1 AS R "
							   "INNER JOIN Pose "
							   "	ON R.poseId = Pose.id "
							   "INNER JOIN File "
							   "	ON Pose.fileId = File.id "
							   "ORDER BY score DESC "
							   "LIMIT ?")
						   .arg(temp0.text()),
					   limit);
	std::vector<int> res;
	while (q.next()) {
		const int id = q.value("id").toInt();
		qDebug() << id << ", " << q.value("score");
		res.push_back(id);
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
