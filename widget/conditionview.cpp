#include "conditionview.hpp"
#include <QContextMenuEvent>
#include <QHeaderView>
#include <QMenu>
#include <QPointer>

void ConditionView::setModel(QAbstractItemModel *model) {
	QTableView::setModel(model);

	if (!model)
		return;

	auto *header = horizontalHeader();
	// 1列目（チェックボックス）は最小限
	header->setSectionResizeMode(0, QHeaderView::ResizeToContents);
	// 2列目は内容に応じて調整
	header->setSectionResizeMode(1, QHeaderView::Interactive);
	// 3列目は可能な限りスペースを与える
	header->setSectionResizeMode(2, QHeaderView::Stretch);
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
