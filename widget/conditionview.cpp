#include "conditionview.hpp"
#include <QContextMenuEvent>
#include <QMenu>
#include <QPointer>

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
