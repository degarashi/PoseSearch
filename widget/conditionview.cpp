#include "conditionview.hpp"
#include <QContextMenuEvent>
#include <QHeaderView>
#include <QMenu>
#include <QPointer>
#include "widget/conditionlist_dlg.hpp"

ConditionView::ConditionView(QWidget *parent) : QTableView(parent) {
	auto *header = horizontalHeader();
	header->setVisible(false);

	setItemDelegate(new SliderDelegate(this));
}

void ConditionView::setModel(QAbstractItemModel *model) {
	QTableView::setModel(model);

	if (!model)
		return;

	auto *header = horizontalHeader();
	header->setSectionResizeMode(0, QHeaderView::ResizeToContents);
	header->setSectionResizeMode(1, QHeaderView::Interactive);
	header->setSectionResizeMode(2, QHeaderView::ResizeToContents);
	header->setSectionResizeMode(3, QHeaderView::Stretch);
}

void ConditionView::contextMenuEvent(QContextMenuEvent *event) {
	const QModelIndex idx = indexAt(event->pos());

	// 選択されていない場合はデフォルト動作
	if (!idx.isValid() || !selectionModel()->isSelected(idx)) {
		QTableView::contextMenuEvent(event);
		return;
	}

	QPointer<QMenu> menu = new QMenu(this);
	auto *editAct = new QAction(tr("Edit Parameter"), menu);
	connect(editAct, &QAction::triggered, this,
			[this, pIndex = QPersistentModelIndex(idx)]() { emit onItemEdit(pIndex); });
	menu->addAction(editAct);
	menu->popup(event->globalPos());
}
