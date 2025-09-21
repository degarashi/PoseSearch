#pragma once
#include <QStringList>
#include <QVector3D>
#include "aux_f/angle.hpp"
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
		// (Debug用) スコア情報
		struct QueryScore {
				float score;
				std::vector<float> individual;
		};
		using QueryResult_V = std::vector<QueryScore>;
		MyDatabase(std::unique_ptr<dg::sql::Database> db);
		const QStringList &getTagList() const;
		QString getTag(int idx) const;
		dg::sql::Database &database() const;
		QString getFilePath(int fileId) const;
		int getFileId(int poseId) const;

		struct PoseInfo {
				std::vector<QVector2D> landmarks;
				QVector3D torsoDir;
				// [0] = left, [1] = right
				QVector3D thighDir[2];
				QVector3D crusDir[2];
				dg::Degree thighFlex[2];
				dg::Degree crusFlex[2];
		};
		// ent.poseIdに関連する姿勢情報を一括取得して返す関数を定義
		PoseInfo getPoseInfo(int poseId) const;

		std::vector<int> query(int limit, const std::vector<Condition *> &clist) const;
		QueryScore getScore(int poseId) const;

	private:
		QStringList _tags;
		std::unique_ptr<dg::sql::Database> _db;
		bool _debugMode;
};
