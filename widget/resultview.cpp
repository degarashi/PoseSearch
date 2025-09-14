#include "resultview.h"
#include <QDrag>
#include <QMimeData>
#include <QUrl>
#include <qevent.h>

ResultView::ResultView(QWidget *parent) : QListView(parent) {
	setDragEnabled(true);
}

void ResultView::startDrag(Qt::DropActions supportedActions) {
	qDebug() << "start drag";
	QModelIndexList indexes = selectedIndexes();
	if (indexes.isEmpty())
		return;

	QMimeData* mime = model()->mimeData(indexes);
	QDrag *drag = new QDrag(this);
	drag->setMimeData(mime);
	drag->exec(Qt::CopyAction);
}
