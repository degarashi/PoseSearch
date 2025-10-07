#pragma once
#include <QSqlDatabase>
#include <QSqlDriver>
#include "aux_f/debug.hpp"
#include "name.hpp"
#include "query.hpp"

namespace dg::sql {
	using FeatureV = std::vector<QSqlDriver::DriverFeature>;
	using PragmaV = std::vector<std::pair<QString, QString>>;
	using QString2V = std::vector<std::tuple<QString, QString>>;
	class Database {
		private:
			QString _name;
			PragmaV _pragma;
			QSqlDatabase _db;
			QSqlQuery _makeSchemaQuery(const Name &target, const QString &column, const QString &type) const;

		public:
			Database(const QString &name, const FeatureV &feature, const PragmaV &pragma);
			Database(const QString &name, const QString &path, const FeatureV &feature, const PragmaV &pragma);
			~Database();
			// --- DB ---
			void setMainDB(const QString &path);
			void attach(const QString &path, const QString &name);
			void detach(const QString &path);
			void loadExtension(const QString &path, const QString &entry_point);
			void open();
			void close();
			QString getSchema(const Name &src) const;
			QString2V getIndex(const Name &src) const;
			const QSqlDatabase &database() const noexcept;
			// --- Transaction ---
			void beginTransaction();
			void commitTransaction();
			void rollbackTransaction();

			// --- Table ---
			void copyTableData(const Name &src, const Name &dst);
			void copyTableDesc(const Name &src, const Name &dst, bool copy_index = true);
			void copyIndex(const Name &src, const Name &dst);
			void renameTable(const Name &src, const QString &dst);
			void dropTable(const Name &table, bool ignoreError = false);
			void clearTable(const Name &table);
			void createTempTable(const QString &tableName, const QString &body, bool ignoreError = false) const;
			int getNTempTable() const;
			bool hasTable(const Name &name) const;

			// --- Query ---
			template <typename... Ts>
			QSqlQuery exec(WithLocation<QString> text, Ts &&...ts) const {
				return ::dg::sql::Query(_db, text, std::forward<Ts>(ts)...);
			}

			template <typename... Ts>
			QSqlQuery batch(WithLocation<QString> text, Ts &&...ts) const {
				return ::dg::sql::Query<&::dg::sql::Batch>(_db, text, std::forward<Ts>(ts)...);
			}
	};
} // namespace dg::sql
