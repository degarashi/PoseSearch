#pragma once
#include <QAbstractItemModel>
#include <QPixmap>
#include <QStringList>
#include "id.hpp"

class ResultPathModel : public QAbstractListModel {
		Q_OBJECT
	public:
		explicit ResultPathModel(QObject *parent = nullptr);

		virtual int rowCount(const QModelIndex &parent) const override;
		virtual QVariant data(const QModelIndex &index, int role) const override;
		Qt::ItemFlags flags(const QModelIndex &index) const override;
		QMimeData *mimeData(const QModelIndexList &indexes) const override;
		QStringList mimeTypes() const override;
		Qt::DropActions supportedDragActions() const override;

		void addIds(const PoseIds &poseIds);
		void clear();

	private:
		struct Entry {
				PoseId poseId;
				FileId fileId;
				QPixmap thumbnail;
		};
		QList<Entry> _data;
};
