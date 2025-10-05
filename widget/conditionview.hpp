#pragma once
#include <QTableView>

class ConditionView : public QTableView {
		Q_OBJECT

	public:
		ConditionView(QWidget *parent);
		void setModel(QAbstractItemModel *model) override;

	protected:
		void contextMenuEvent(QContextMenuEvent *event) override;

	signals:
		void onItemEdit(const QModelIndex &index);
};
