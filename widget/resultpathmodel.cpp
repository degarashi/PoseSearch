#include "resultpathmodel.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QMimeData>
#include <QUrl>
#include "aux_f/convert.hpp"
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
				case Qt::ToolTipRole: {
					const auto info = myDb_c.getPoseInfo(ent.poseId);
					const auto sc = myDb_c.getScore(ent.poseId);
					// カーソルホバー時に表示する文字列
					auto msg = QString("%1\nScore: %2").arg(myDb_c.getFilePath(ent.fileId)).arg(sc.score);
					for (auto &&scl : sc.individual)
						msg += QString("\n\t%1").arg(scl);

					msg += "\n--TorsoDir--\n";
					msg += dg::VecToString(info.torsoDir);
					msg += "\n--ThighDir--\n";
					msg += dg::VecToString(info.thighDir[0]);
					msg += dg::VecToString(info.thighDir[1]);
					msg += "\n--CrusDir--\n";
					msg += dg::VecToString(info.crusDir[0]);
					msg += dg::VecToString(info.crusDir[1]);
					msg += "\n--ThighFlex--\n";
					msg += QString("%1, %2").arg(info.thighFlex[0].toString()).arg(info.thighFlex[1].toString());

					return msg;
				}
				case Qt::DecorationRole:
					return ent.thumbnail;
				case Qt::UserRole:
					return ent.poseId;
				default:;
			}
		}
	}
	return {};
}

void ResultPathModel::addIds(const std::vector<int> &poseIds) {
	const int count = poseIds.size();
	if (count == 0)
		return;

	std::vector<int> fileIds;
	for (const auto poseId : poseIds)
		fileIds.emplace_back(myDb_c.getFileId(poseId));

	const auto thumbnails = myTn.getThumbnails(fileIds);
	Q_ASSERT(thumbnails.size() == count);

	beginInsertRows(QModelIndex(), _data.size(), _data.size() + count - 1);
	for (int i = 0; i < count; ++i)
		_data.append(Entry{poseIds[i], fileIds[i], thumbnails[i]});
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
