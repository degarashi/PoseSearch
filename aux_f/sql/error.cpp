#include "error.hpp"
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStringBuilder>
#include "exception.hpp"

namespace dg::sql {
	void ThrowError(QSqlDatabase &db, const QString &message) {
		throw std::runtime_error(QString(message % '\n' % db.lastError().text()).toStdString());
	}
	void ThrowError(QSqlQuery &q) {
		throw ExecutionError(q);
	}
} // namespace dg::sql
