#include "exception.hpp"
#include <QRegularExpression>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

namespace dg::sql {
	namespace {
		const std::string c_featureName[] = {
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
		RuntimeError("feature not supported: " + c_featureName[num]), _num(num) {
	}
	// ------------------- CantOpenDatabase -------------------
	CantOpenDatabase::CantOpenDatabase(const std::string &path) : RuntimeError("can't open database (" + path + ")") {
	}
	// ------------------- CantLoadExtension -------------------
	CantLoadExtension::CantLoadExtension(const std::string &path) :
		RuntimeError("can't load extension: path(" + path + ")") {
	}
	// ------------------- ExecutionError -------------------
	namespace {
		void RemoveLeadingSpaces(std::string &text) {
			// ^\s+   : 行頭の空白（スペースやタブなど）1文字以上
			// (?m)   : 複数行モード（^ と $ が行頭・行末にマッチする）
			QRegularExpression re(QStringLiteral("(?m)^\\s+"));
			QString qtext = QString::fromStdString(text);
			qtext.replace(re, "");
			text = qtext.toStdString();
		}
		std::string PrettyPrintQuery(const std::string &query) {
			std::string prettyQuery = query;
			RemoveLeadingSpaces(prettyQuery);
			return prettyQuery;
		}

		std::string PrettyPrintValues(const QSqlQuery &query) {
			std::string ret;

			// [key: value]
			if (!query.boundValueNames().empty()) {
				ret += "\n---(key:value)---\n";
				for (const auto &key : query.boundValueNames()) {
					const auto &value = query.boundValue(key);
					ret += key.toStdString() + ": " + value.toString().toStdString() + "\n";
				}
			}
			if (!query.boundValues().empty()) {
				ret += "\n---(positional)---\n";
				// Indexed Value
				for (const auto &value : query.boundValues()) {
					ret += value.toString().toStdString();
					ret += "\n";
				}
			}
			return ret;
		}
	} // namespace
	ExecutionError::ExecutionError(const QSqlQuery &query) :
		RuntimeError("execution error: " + query.lastError().text().toStdString() + "\nquery: " +
					 PrettyPrintQuery(query.lastQuery().toStdString()) + "\nvalues: " + PrettyPrintValues(query)) {
	}

} // namespace dg::sql
