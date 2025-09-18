#include "database.hpp"
#include <QDir>
#include <QRegularExpression>
#include <QSqlError>
#include "exception.hpp"
#include "getvalues.hpp"
#include "query.hpp"

namespace dg::sql {
	namespace {
		const QString c_dbType("QSQLITE");
	}
	Database::Database(const QString &name, const QString &path, const FeatureV &feature, const PragmaV &pragma) :
		Database(name, feature, pragma) {
		setMainDB(path);
		open();
	}
	Database::Database(const QString &name, const FeatureV &feature, const PragmaV &pragma) :
		_name(name), _pragma(pragma), _db(QSqlDatabase::addDatabase(c_dbType, name)) {
		// featureの確認
		for (auto &&f : feature) {
			if (!_db.driver()->hasFeature(f))
				throw FeatureNotSupported(f);
		}
	}
	void Database::createTempTable(const QString &tableName, const bool ignoreError) const {
		exec(QString("CREATE TEMPORARY TABLE %2 %1 ("
					 "poseId INTEGER NOT NULL,"
					 "score REAL NOT NULL"
					 ")")
				 .arg(tableName, ignoreError ? "IF NOT EXISTS" : ""));
	}

	int Database::getNTempTable() const {
		auto q = exec("SELECT COUNT(*) FROM sqlite_temp_master");
		if (q.next())
			return q.value(0).toInt();
		return 0;
	}
	bool Database::hasTable(const Name &name) const {
		auto q =
			exec(QString("SELECT name FROM %1.sqlite_master WHERE type='table' AND name=?").arg(name.db), name.table);
		return q.next();
	}
	void Database::setMainDB(const QString &path) {
		_db.setDatabaseName(QDir(path).absolutePath());
	}
	void Database::attach(const QString &path, const QString &name) {
		sql::Query(_db, QString("ATTACH DATABASE ? AS ?"), QDir(path).absolutePath(), name);
	}
	QSqlQuery Database::_makeSchemaQuery(const Name &target, const QString &column, const QString &type) const {
		return sql::Query(_db,
						  QString("SELECT %1 FROM %2 WHERE tbl_name=? AND type=? AND sql IS NOT NULL")
							  .arg(column)
							  .arg(target.withTable("sqlite_master")),
						  target.table, type);
	}
	QString2V Database::getIndex(const Name &src) const {
		return sql::GetTupleValues<QString, QString>(
			_makeSchemaQuery(src, QStringLiteral("name, sql"), QStringLiteral("index")));
	}
	QString Database::getSchema(const Name &src) const {
		return sql::GetRequiredValue<QString>(_makeSchemaQuery(src, QStringLiteral("sql"), QStringLiteral("table")));
	}
	void Database::detach(const QString &path) {
		exec(QString("DETACH DATABASE ?"), path);
	}
	void Database::open() {
		if (!_db.isOpen()) {
			if (!_db.open())
				throw CantOpenDatabase(_db.lastError().text());
			// pragmaの適用
			for (auto &&p : _pragma) {
				exec(QString("PRAGMA %1 = %2").arg(p.first, p.second));
			}
		}
	}
	void Database::close() {
		if (_db.isOpen()) {
			_db.close();
		}
	}
	void Database::copyTableDesc(const Name &src, const Name &dst, const bool copy_index) {
		{
			// 定義をコピー
			auto db = database();
			auto sc = getSchema(src);
			// テーブル名が""で囲まれていたらこれを外す
			sc.replace(QRegularExpression(QString(R"-("(%1)")-").arg(src.table)), R"(\1)");
			sc.replace(src.table, dst.text());
			sql::Query(db, sc);
		}
		// インデックスもコピー
		if (copy_index)
			copyIndex(dst, src);
	}
	void Database::copyIndex(const Name &src, const Name &dst) {
		auto db = database();
		for (auto &&idx : getIndex(src)) {
			auto &indexName = std::get<0>(idx);
			auto &schema = std::get<1>(idx);
			schema.replace(indexName, dst.db + indexName);
			sql::Query(db, schema);
		}
	}
	void Database::copyTableData(const Name &src, const Name &dst) {
		sql::Query(database(),
				   QString("INSERT INTO %1\n"
						   "	SELECT * FROM %2\n")
					   .arg(dst.text(), src.text()));
	}
	const QSqlDatabase &Database::database() const noexcept {
		return _db;
	}
	Database::~Database() {
		if (std::uncaught_exceptions() == 0) {
			close();
			_db = QSqlDatabase();
			QSqlDatabase::removeDatabase(_name);
		}
	}
	void Database::renameTable(const Name &src, const QString &dst) {
		sql::Query(database(), QString("ALTER TABLE %1 RENAME TO %2").arg(src.text(), dst));
	}
	void Database::dropTable(const Name &table, const bool ignoreError) {
		sql::Query(database(), QString("DROP TABLE %1 %2").arg(ignoreError ? "IF EXISTS" : "", table.text()));
	}
	void Database::clearTable(const Name &table) {
		const QString TempName("dg_temp_table");
		copyTableDesc({table.db, TempName}, table, true);
		dropTable(table);
		renameTable({table.db, TempName}, table.table);
	}
	void Database::loadExtension(const QString &path, const QString &entry_point) {
		try {
			exec(QString("SELECT load_extension('%1', '%2')").arg(path, entry_point));
		}
		catch (const ExecutionError &e) {
			throw CantLoadExtension(e.q_what());
		}
	}
	void Database::beginTransaction() {
		exec("BEGIN TRANSACTION");
	}
	void Database::commitTransaction() {
		exec("COMMIT TRANSACTION");
	}
	void Database::rollbackTransaction() {
		exec("ROLLBACK TRANSACTION");
	}
} // namespace dg::sql
