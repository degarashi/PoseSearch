#pragma once
#include <QListView>

class ConditionListView : public QListView {
		Q_OBJECT

	public:
		using QListView::QListView;

	protected:
		void contextMenuEvent(QContextMenuEvent *event) override;

	signals:
		void onItemEdit(const QModelIndex &index);
};
