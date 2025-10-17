#pragma once
#include <QStringList>
#include <QVector3D>
#include "aux_f_q/sql/database.hpp"
#include "id.hpp"
#include "poseinfo.hpp"
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

		// コンストラクタ
		MyDatabase(std::unique_ptr<dg::sql::Database> db);

		// データベースアクセサ
		dg::sql::Database &database() const;

		// タグ関連
		const QStringList &getTagList() const;
		QString getTag(int idx) const;

		// ファイル関連
		QString getFilePath(FileId fileId) const;
		FileId getFileId(PoseId poseId) const;
		QByteArray getFileHash(FileId fileId) const;

		// ポーズ関連
		QRectF getPoseRect(PoseId poseId) const;
		PoseInfo getPoseInfo(PoseId poseId) const;
		QueryScore getScore(PoseId poseId) const;

		// クエリ関連
		PoseIds query(int limit, const std::vector<Condition *> &clist) const;

		// ブラックリスト関連
		void addBlacklist(FileId fileId) const;
		void removeBlacklist(FileId fileId) const;
		bool isBlacklisted(FileId fileId) const;
		void deleteBlacklist();

		bool usingPartialHash() const;

		size_t getNImages() const;
		size_t getNPoses() const;
	private:
		QStringList _tags;
		std::unique_ptr<dg::sql::Database> _db;
		bool _debugMode;
		bool _usePartialHash;
};
