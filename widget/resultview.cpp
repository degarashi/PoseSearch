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
#include "aux_f_q/q_value.hpp"
#include "poseinfodialog.h"
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
	Q_ASSERT(pIndex.isValid());
	const int poseId = dg::ConvertQV<int>(pIndex.data(Qt::UserRole));

	QPointer<QMenu> menu = new QMenu(this);
	menu->setAttribute(Qt::WA_DeleteOnClose);

	// --- ファイルパスコピー ---
	auto *copyPathAction = new QAction(tr("Copy FilePath"), menu);
	connect(copyPathAction, &QAction::triggered, this, [pIndex]() {
		const int fileId = pIndex.data(Qt::UserRole).toInt();
		const QString filePath = myDb_c.getFilePath(fileId);
		QApplication::clipboard()->setText(filePath);
	});
	menu->addAction(copyPathAction);

	// --- PoseInfoDialog表示 ---
	auto *showPoseInfoAction = new QAction(tr("Show PoseInfo"), menu);
	connect(showPoseInfoAction, &QAction::triggered, this, [this, pIndex, poseId]() {
		// PoseInfoDialog を生成して表示
		auto *dialog = new PoseInfoDialog(poseId, this);
		dialog->setAttribute(Qt::WA_DeleteOnClose);
		dialog->show();
	});
	menu->addAction(showPoseInfoAction);

	menu->addSeparator();

	// --- ブラックリスト登録/解除 ---
	const bool isBlacklisted = myDb.isBlacklisted(poseId);
	auto *blacklistAction = new QAction(isBlacklisted ? tr("Remove Blacklist") : tr("Add Blacklist"), menu);
	if (isBlacklisted)
		connect(blacklistAction, &QAction::triggered, this, [poseId]() { myDb.removeBlacklist(poseId); });
	else
		connect(blacklistAction, &QAction::triggered, this, [poseId]() { myDb.addBlacklist(poseId); });
	menu->addAction(blacklistAction);

	menu->popup(event->globalPos());
}
