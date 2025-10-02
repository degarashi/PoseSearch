#include "my_thumbnail.hpp"
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QImage>
#include <QMessageBox>
#include <QSqlError>
#include <QtConcurrent/QtConcurrent>
#include "aux_f/exception.hpp"
#include "aux_f_q/sql/database.hpp"
#include "my_db.hpp"

namespace {
	static const auto THUMBNAIL_DIR = QStringLiteral("thumbnail");
	static const auto THUMBNAIL_DB = THUMBNAIL_DIR + "/" + "thumbnail.sqlite3";
	static const auto THUMB_DB = QStringLiteral("thumb");
	static const auto THUMB_TABLE = dg::sql::Name(THUMB_DB, "Thumbnail");
	constexpr int IconSize = 64;

	// ファイルパスからキャッシュファイル名を計算
	QString CalculateCacheName(const QString &filePath) {
		QFile file(filePath);
		if (!file.open(QIODevice::ReadOnly)) {
			throw dg::CantOpenFile(filePath.toStdString());
		}
		// Blake2b_256ハッシュを計算
		QCryptographicHash hash(QCryptographicHash::Blake2b_256);
		hash.addData(&file);
		const QByteArray hashBytes = hash.result();
		// ハッシュ値を16進数文字列に変換
		const QString hashString = QString(hashBytes.toHex());
		file.close();
		// ".png"拡張子を付けて返す
		return hashString + ".png";
	}
} // namespace

MyThumbnail::MyThumbnail() {
	auto &db = myDb.database();
	// THUMBNAIL_DBの場所にディレクトリがなければ作る
	QDir thumbnailDir(THUMBNAIL_DIR);
	if (!thumbnailDir.exists()) {
		if (!thumbnailDir.mkpath(".")) {
			throw dg::RuntimeError(
				QString("Failed to create thumbnail directory: %1").arg(THUMBNAIL_DIR).toStdString());
		}
	}
	db.attach(THUMBNAIL_DB, THUMB_DB);
	if (!db.hasTable(THUMB_TABLE)) {
		db.exec("CREATE TABLE thumb.Thumbnail ( "
				"	fileId		INTEGER PRIMARY KEY, "
				"	cacheName	TEXT NOT NULL UNIQUE "
				"); ");
	}
}

std::vector<QPixmap> MyThumbnail::getThumbnails(const std::vector<int> &fileIds) {
	if (fileIds.empty())
		return {};

	struct WItem {
			int index;
			int fileId;
			QString filePath;
			QPixmap thumbnail;
			QString cacheFileName;

			WItem(const int index_v, const int fileId_v, const QString &filePath_v, const QPixmap &pm = {}) :
				index(index_v), fileId(fileId_v), filePath(filePath_v), thumbnail(pm) {
			}
	};
	// サムネイル生成処理
	const auto workerFunc = [](WItem &item) {
		try {
			std::tie(item.thumbnail, item.cacheFileName) = _GenerateThumbnail(item.filePath, item.fileId);
		}
		catch (const dg::RuntimeError &e) {
			qDebug() << "Error generating thumbnail for file-id:" << item.fileId << e.what();
		}
	};
	// 最終的に統合するアイテム
	std::vector<WItem> result;
	// 並列処理するアイテム
	std::vector<WItem> wItem;

	auto &db = myDb.database();
	int index = 0;
	// キャッシュを確認して、生成の必要がある物を洗い出す
	for (const int fileId : fileIds) {
		// キャッシュがあるか確認
		auto q = db.exec("SELECT File.path, Thumbnail.cacheName "
						 "FROM main.File "
						 "LEFT JOIN thumb.Thumbnail "
						 "	ON File.id = Thumbnail.fileId "
						 "WHERE File.id = ? ",
						 fileId);

		q.next();

		const QString filePath = q.value(0).toString();
		Q_ASSERT(!filePath.isEmpty());

		bool cacheUsed = false;
		if (!q.value(1).isNull()) {
			const auto cachedFileName = q.value(1).toString();
			// tPathがサムネイルのファイルパスなので、これを読み取りQPixmapにして返す
			QPixmap thumbnail;
			thumbnail.load(THUMBNAIL_DIR + "/" + cachedFileName);
			if (!thumbnail.isNull()) {
				result.emplace_back(index, fileId, filePath, std::move(thumbnail));
				cacheUsed = true;
			}
			else {
				// キャッシュファイルが破損している場合は再生成
				qDebug() << "Thumbnail cache corrupted for fileId:" << fileId << "cacheName:" << cachedFileName;
			}
		}
		if (!cacheUsed) {
			// 処理予約
			wItem.emplace_back(index, fileId, myDb_c.getFilePath(fileId));
		}
		++index;
	}
	if (!wItem.empty()) {
		// 並列処理
		QFuture<void> future = QtConcurrent::map(wItem, workerFunc);
		future.waitForFinished();

		// 生成されたサムネイルをDBに登録
		std::vector<int> generatedFileIds;
		QStringList generatedCacheName;
		for (const WItem &item : wItem) {
			// 生成に成功した場合のみDBに登録
			if (!item.thumbnail.isNull()) {
				generatedFileIds.emplace_back(item.fileId);
				generatedCacheName.emplace_back(item.cacheFileName);
			}
		}
		_registerThumbnails(generatedFileIds, generatedCacheName);

		// キャッシュがあった分と統合する
		for (auto &&item : wItem)
			result.emplace_back(std::move(item));

		// indexでソート
		std::sort(result.begin(), result.end(), [](const WItem &a, const WItem &b) { return a.index < b.index; });
	}
	// vectorに詰め直す
	std::vector<QPixmap> ret;
	ret.reserve(fileIds.size());
	for (const WItem &res : result)
		ret.emplace_back(std::move(res.thumbnail));

	return ret;
}

std::pair<QPixmap, QString> MyThumbnail::_GenerateThumbnail(const QString &filePath, const int fileId) {
	if (fileId <= 0) {
		throw dg::InvalidInput(std::string("Invalid fileId ") + std::to_string(fileId));
	}

	QFile imgFile(filePath);
	if (!imgFile.open(QIODevice::ReadOnly))
		throw dg::CantOpenFile(filePath.toStdString());

	// 画像作ってリサイズ
	QImage img;
	const auto imgData = imgFile.readAll();
	if (imgData.length() == 0)
		// データ長が0の場合はエラー
		throw dg::UnknownImage(filePath.toStdString());
	img = img.fromData(imgData);
	img = img.scaled(IconSize, IconSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	if (img.isNull()) {
		throw dg::CantMakeThumbnail("resizing image is failed");
	}
	// リサイズした画像からサムネイルを作成
	QPixmap ret;
	ret = ret.fromImage(img);

	const QString cacheName = CalculateCacheName(filePath);
	const QString cachePath(THUMBNAIL_DIR + "/" + cacheName);

	// サムネイルをキャッシュディレクトリに保存
	if (ret.isNull())
		throw dg::CantMakeThumbnail(filePath.toStdString());

	QDir dir;
	if (!dir.exists(THUMBNAIL_DIR)) {
		if (!dir.mkpath(THUMBNAIL_DIR)) {
			throw dg::RuntimeError(QString("Failed to mkdir \"%1\"").arg(THUMBNAIL_DIR).toStdString());
		}
	}
	if (!ret.save(cachePath, "PNG")) {
		throw dg::RuntimeError(QString("Failed to save thumbnail \"%1\"").arg(cachePath).toStdString());
	}
	return std::make_tuple(std::move(ret), cacheName);
}

void MyThumbnail::_registerThumbnails(std::vector<int> fileIds, const QStringList &cacheNames) {
	if (fileIds.empty())
		return;

	// データベースにキャッシュ情報を保存または更新
	auto &db = myDb.database();
	db.beginTransaction();
	const auto q = db.batch("INSERT INTO thumb.Thumbnail (fileId, cacheName) VALUES (?,?)", fileIds, cacheNames);
	QSqlError err = q.lastError();
	if (err.isValid()) {
		qDebug() << "Database error during thumbnail registration:" << err.text();
	}
	db.commitTransaction();
	qDebug() << QString("Thumbnail register: (%1) files").arg(fileIds.size());
}

void MyThumbnail::clearThumbnail() {
	// THUMBNAIL_DIR内のpngファイルを全て消去
	QDir dir(THUMBNAIL_DIR);
	int removedCount = 0;
	if (dir.exists()) {
		const QStringList pngFiles = dir.entryList(QStringList() << "*.png", QDir::Files);
		for (const QString &file : pngFiles) {
			if (!dir.remove(file)) {
				qWarning() << "Failed to remove thumbnail file:" << dir.filePath(file);
			}
			else {
				removedCount++; // 削除に成功したらカウントを増やす
			}
		}
	}
	// データベースからも関連情報を削除
	auto &db = myDb.database();
	db.exec("DELETE FROM thumb.Thumbnail");
	// 削除した件数をQMessageBoxで表示
	QMessageBox::information(nullptr, "Thumbnail Cleared", QString("Removed %1 thumbnail files.").arg(removedCount));
}
