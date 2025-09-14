#pragma once
#include "../exception.hpp"

QT_BEGIN_NAMESPACE
class QSqlQuery;
QT_END_NAMESPACE

namespace dg::sql {
	class FeatureNotSupported : public ::dg::RuntimeError {
			Q_OBJECT
		private:
			int _num;

		public:
			explicit FeatureNotSupported(int num);
	};
	class CantOpenDatabase : public ::dg::RuntimeError {
			Q_OBJECT
		public:
			CantOpenDatabase(const QString &path);
	};
	class CantLoadExtension : public ::dg::RuntimeError {
			Q_OBJECT
		public:
			CantLoadExtension(const QString &path);
	};
	class ExecutionError : public ::dg::RuntimeError {
			Q_OBJECT
		public:
			ExecutionError(const QSqlQuery &query);
	};
} // namespace dg::sql
