#include "resultview.h"
#include <QDrag>
#include <QMimeData>
#include <QUrl>
#include <qevent.h>

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
