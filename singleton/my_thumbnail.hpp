#pragma once
#include <QMap>
#include <QPixmap>
#include "id.hpp"
#include "singleton.hpp"

namespace dg::sql {
	class Database;
}

#define myTn (MyThumbnail::Get())

// とりあえず指定サイズのサムネイルだけ担当
class MyThumbnail : public dg::Singleton<MyThumbnail> {
	public:
		MyThumbnail();
		void clearThumbnail();
		std::vector<QPixmap> getThumbnails(const FileIds &fileIds);

	private:
		static std::pair<QPixmap, QString> _GenerateThumbnail(const QString &filePath, FileId fileId);

		void _registerThumbnails(const FileIds &fileIds, const QStringList &cacheName);
};
