#pragma once
#include <QStringList>
#include "aux_f/sql/database.hpp"
#include "singleton.hpp"

class Condition;

namespace dg {
	void LoadVecExtension(dg::sql::Database &db);
}

#define myDb (MyDatabase::Get())
#define myDb_c (MyDatabase::GetC())

class MyDatabase : public dg::Singleton<MyDatabase> {
	public:
		MyDatabase(std::unique_ptr<dg::sql::Database> db);
		const QStringList &getTagList() const;
		QString getTag(int idx) const;
		dg::sql::Database& database() const;
		QString getFilePath(int fileId) const;

		std::vector<int> query(int limit, const std::vector<Condition*> &clist) const;

	private:
		QStringList _tags;
		std::unique_ptr<dg::sql::Database> _db;
};
