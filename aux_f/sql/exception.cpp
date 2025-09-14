#include "exception.hpp"
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

namespace dg::sql {
	namespace {
		const QString c_featureName[] = {
			"Transactions",
			"QuerySize",
			"BLOB",
			"Unicode",
			"PreparedQueries",
			"NamedPlaceholders",
			"PositionalPlaceholders",
			"LastInsertId",
			"BatchOperations",
			"SimpleLocking",
			"LowPrecisionNumbers",
			"EventNotifications",
			"FinishQuery",
			"MultipleResultSets",
			"CancelQuery",
		};
	}
	FeatureNotSupported::FeatureNotSupported(const int num) :
		RuntimeError(tr("feature not supported: %1").arg(c_featureName[num])), _num(num) {
	}
	// ------------------- CantOpenDatabase -------------------
	CantOpenDatabase::CantOpenDatabase(const QString &path) : RuntimeError(tr("can't open database (%1)").arg(path)) {
	}
	// ------------------- CantLoadExtension -------------------
	CantLoadExtension::CantLoadExtension(const QString &path) :
		RuntimeError(tr("can't open extension path(%1)").arg(path)) {
	}
	// ------------------- ExecutionError -------------------
	namespace {

		QString PrettyPrintQuery(const QString &query) {
			QString prettyQuery = query;
			prettyQuery.replace("(", "(\n");
			prettyQuery.replace(", ", ",\n");
			return prettyQuery;
		}

		QString PrettyPrintValues(const QSqlQuery &query) {
			QString ret;

			// [key: value]
			if (!query.boundValueNames().empty()) {
				ret += QStringLiteral("\n---(key:value)---\n");
				for (const auto &key : query.boundValueNames()) {
					const auto &value = query.boundValue(key);
					ret += QStringLiteral("%1: %2\n").arg(key, value.toString());
				}
			}
			if (!query.boundValues().empty()) {
				ret += QStringLiteral("\n---(positional)---\n");
				// Indexed Value
				for (const auto &value : query.boundValues()) {
					ret += value.toString();
					ret += "\n";
				}
			}
			return ret;
		}
	} // namespace
	ExecutionError::ExecutionError(const QSqlQuery &query) :
		// DONE: queryに使われたパラメータを文字列にして連結し、出力
		RuntimeError(
			tr("execution error: %1\nquery: %2\nvalues: %3")
				.arg(query.lastError().text(), PrettyPrintQuery(query.lastQuery()), PrettyPrintValues(query))) {
	}

} // namespace dg::sql
