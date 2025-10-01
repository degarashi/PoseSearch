#pragma once
#include "../exception.hpp"

class QSqlQuery;

namespace dg::sql {
	class FeatureNotSupported : public ::dg::RuntimeError {
		private:
			int _num;

		public:
			explicit FeatureNotSupported(int num);
	};
	class CantOpenDatabase : public ::dg::RuntimeError {
		public:
			CantOpenDatabase(const std::string &path);
	};
	class CantLoadExtension : public ::dg::RuntimeError {
		public:
			CantLoadExtension(const std::string &path);
	};
	class ExecutionError : public ::dg::RuntimeError {
		public:
			ExecutionError(const QSqlQuery &query);
	};
} // namespace dg::sql
