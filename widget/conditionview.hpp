#pragma once
#include <QTableView>

class ConditionView : public QTableView {
		Q_OBJECT

	public:
		using QTableView::QTableView;

	protected:
		void contextMenuEvent(QContextMenuEvent *event) override;

	signals:
		void onItemEdit(const QModelIndex &index);
};
