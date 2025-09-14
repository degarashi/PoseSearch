#pragma once
#include <QString>

namespace dg::sql {
	struct Name {
			QString db, table;

			Name(const QString &table_a) : table(table_a) {
			}
			Name(const QString &db_a, const QString &table_a) : db(db_a), table(table_a) {
			}
			QString withTable(const QString &tbl) const {
				if (db.isNull())
					return tbl;
				return db + '.' + tbl;
			}
			QString withDB(const QString &dbs) const {
				return dbs + '.' + table;
			}
			QString text() const {
				if (db.isNull())
					return table;
				return db + '.' + table;
			}
	};
} // namespace dg::sql
