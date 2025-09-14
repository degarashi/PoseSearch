#pragma once
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QVariant>

class QString;
namespace dg::sql {
	QString MakeQueryString(const QSqlQuery &q);
	void Query(QSqlQuery &q);
	void Batch(QSqlQuery &q);
	using Proc_p = void (*)(QSqlQuery &);

	namespace detail {
		template <bool, Proc_p>
		void AddBind(QSqlQuery &) {
		}

		template <bool WithNull, Proc_p Proc, class Value, class... Args>
		void AddBind(QSqlQuery &q, const Value &value, const Args &...args) {
			const auto v = QVariant::fromValue(value);
			if (WithNull || !v.isNull())
				q.addBindValue(v);
			AddBind<WithNull, Proc>(q, args...);
		}

		template <bool WithNull, Proc_p Proc, class... Args>
		QSqlQuery Query(const QSqlDatabase &db, const QString &str, const Args &...args) {
			QSqlQuery q(db);
			q.prepare(str);
			AddBind<WithNull, Proc>(q, args...);
			Proc(q);
			return std::move(q);
		}
	} // namespace detail

	template <Proc_p Proc = &Query, typename... Args>
	QSqlQuery Query(const QSqlDatabase &db, const QString &str, const Args &...args) {
		return detail::Query<false, Proc>(db, str, args...);
	}
	template <Proc_p Proc = &Query, typename... Args>
	QSqlQuery QueryWithNull(const QSqlDatabase &db, const QString &str, const Args &...args) {
		return detail::Query<true, Proc>(db, str, args...);
	}
} // namespace dg::sql
