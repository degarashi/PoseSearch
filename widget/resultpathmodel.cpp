#include "resultpathmodel.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QMimeData>
#include <QUrl>
#include "singleton/my_db.hpp"
#include "singleton/my_thumbnail.hpp"

// ---------------------------- ResultPathModel ------------------------------
ResultPathModel::ResultPathModel(QObject *parent) : QAbstractListModel(parent) {
}

int ResultPathModel::rowCount(const QModelIndex &parent) const {
	if (parent.isValid())
		return {};
	return _data.size();
}

QVariant ResultPathModel::data(const QModelIndex &index, const int role) const {
	if (index.isValid()) {
		// 行数がデータ数内
		if (index.row() < _data.size()) {
			const auto &ent = _data[index.row()];
			switch (role) {
				case Qt::ToolTipRole:
					return myDb_c.getFilePath(ent.id);
				case Qt::DecorationRole:
					return ent.thumbnail;
				case Qt::UserRole:
					return ent.id;
				default:;
			}
		}
	}
	return {};
}

void ResultPathModel::addIds(const std::vector<int> &fileIds) {
	const int count = fileIds.size();
	if (count == 0)
		return;

	const auto thumbnails = myTn.getThumbnails(fileIds);
	Q_ASSERT(thumbnails.size() == count);

	beginInsertRows(QModelIndex(), _data.size(), _data.size() + count - 1);
	for (int i = 0; i < count; ++i)
		_data.append(Entry{fileIds[i], thumbnails[i]});
	endInsertRows();
}

void ResultPathModel::clear() {
	beginResetModel();
	_data.clear();
	endResetModel();
}

Qt::DropActions ResultPathModel::supportedDragActions() const {
	return Qt::CopyAction;
}

Qt::ItemFlags ResultPathModel::flags(const QModelIndex &index) const {
	return Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled;
}

QMimeData *ResultPathModel::mimeData(const QModelIndexList &indexes) const {
	QMimeData *mimeData = new QMimeData();
	QList<QUrl> urls;
	for (const QModelIndex &index : indexes) {
		const int fileId = index.data(Qt::UserRole).toInt();
		const QString filePath = myDb.getFilePath(fileId);
		if (!filePath.isEmpty())
			urls << QUrl::fromLocalFile(filePath);
	}
	mimeData->setUrls(urls);
	return mimeData;
}

QStringList ResultPathModel::mimeTypes() const {
	return {"text/plain"};
}
