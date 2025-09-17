#pragma once
#include <QMap>
#include <QPixmap>
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
		std::vector<QPixmap> getThumbnails(const std::vector<int>& fileIds);

	private:
		static std::pair<QPixmap, QString> _GenerateThumbnail(const QString &filePath, int fileId);

		void _registerThumbnails(std::vector<int> fileIds, const QStringList& cacheName);
};
