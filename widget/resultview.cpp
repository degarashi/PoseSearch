#include "resultview.h"
#include <QApplication>
#include <QClipboard>
#include <QContextMenuEvent>
#include <QDebug>
#include <QDrag>
#include <QEvent>
#include <QMenu>
#include <QMimeData>
#include <QPointer>
#include <QUrl>
#include "singleton/my_db.hpp"

ResultView::ResultView(QWidget *parent) : QListView(parent) {
	setDragEnabled(true);
}

void ResultView::startDrag(Qt::DropActions supportedActions) {
	Q_UNUSED(supportedActions);
	qDebug() << "start drag";

	const QModelIndexList indexes = selectedIndexes();
	if (indexes.isEmpty())
		return;

	QMimeData *mime = model()->mimeData(indexes);
	if (!mime)
		return;

	auto *drag = new QDrag(this);
	drag->setMimeData(mime);
	drag->exec(Qt::CopyAction);
}

void ResultView::contextMenuEvent(QContextMenuEvent *event) {
	const QModelIndex idx = indexAt(event->pos());

	// 選択されていない場合はデフォルト動作
	if (!idx.isValid() || !selectionModel()->isSelected(idx)) {
		QListView::contextMenuEvent(event);
		return;
	}

	// 安全な永続インデックス
	const QPersistentModelIndex pIndex(idx);

	QPointer<QMenu> menu = new QMenu(this);
	menu->setAttribute(Qt::WA_DeleteOnClose);

	auto *copyPathAction = new QAction(tr("Copy FilePath"), menu);
	connect(copyPathAction, &QAction::triggered, this, [pIndex]() {
		if (!pIndex.isValid())
			return;
		const int fileId = pIndex.data(Qt::UserRole).toInt();
		const QString filePath = myDb_c.getFilePath(fileId);
		QApplication::clipboard()->setText(filePath);
	});

	menu->addAction(copyPathAction);
	menu->popup(event->globalPos());
}
