#include "mainwindow.h"
#include <QDesktopServices>
#include <QMessageBox>
#include <QSqlError>
#include <aux_f_q/sql/database.hpp>
#include "./ui_mainwindow.h"
#include "aux_f_q/q_value.hpp"
#include "aux_f_q/sql/query.hpp"
#include "condition/condition.hpp"
#include "param/querydialog.h"
#include "singleton/my_db.hpp"
#include "singleton/my_settings.hpp"
#include "singleton/my_thumbnail.hpp"
#include "widget/conditionlistmodel.hpp"
#include "widget/resultpathmodel.h"

// Conditionを生成する為のclass default object
Condition *g_conds[] = {
	&Cond_BodyDir::StaticClass(),
	&Cond_Tag::StaticClass(),
	&Cond_BodyDirYaw::StaticClass(),
	&Cond_BodyDirPitch::StaticClass(),
	&Cond_ThighFlexion::StaticClass(),
	&Cond_CrusFlexion::StaticClass(),
};

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), _ui(new Ui::MainWindow) {
	_ui->setupUi(this);

	_rpm = new ResultPathModel(this);
	_ui->lvResult->setModel(_rpm);

	// 条件リストモデルの作成
	_setConditionModel(std::make_shared<ConditionListModel>(this));

	// 対応するフィルタをComboBoxに設定
	for (auto *cond : g_conds)
		_ui->cbQuery->addItem(cond->dialogName());
}
void MainWindow::_setConditionModel(Cond_SP clm) {
	_ui->lvQueryView->setModel(nullptr);
	_clm = clm;
	_ui->lvQueryView->setModel(_clm.get());

	// 条件リストが更新されたら通知(checkCondition)
	const auto *clm_p = _clm.get();
	connect(clm_p, &QAbstractItemModel::rowsRemoved, this, &MainWindow::checkCondition);
	connect(clm_p, &QAbstractItemModel::rowsInserted, this, &MainWindow::checkCondition);
	connect(clm_p, &QAbstractItemModel::modelReset, this, &MainWindow::checkCondition);
	connect(clm_p, &QAbstractItemModel::dataChanged, this, &MainWindow::checkCondition);

	checkCondition();
}

void MainWindow::query() {
	// 結果モデルをクリア
	_rpm->clear();
	// 条件リストモデルの内容が空だったら何もしない
	auto &cs = _clm->data();
	if (cs.empty()) {
		QMessageBox::information(this, "Information", "No search conditions are set.");
		return;
	}

	// 結果を表示
	const auto sz = cs.size();
	std::vector<Condition *> input;
	for (int i = 0; i < sz; ++i) {
		if (cs[i].enabled)
			input.emplace_back(cs[i].cond.get());
	}
	Q_ASSERT(!input.empty());

	const auto limit = _ui->sboxLimit->value();
	_rpm->addIds(myDb_c.query(limit, input));
}

void MainWindow::addCondition() {
	// ComboBoxから選択されたデータ取得
	const int condIndex = _ui->cbQuery->currentIndex();
	Q_ASSERT(condIndex < std::size(g_conds));
	// クエリダイアログの作成
	auto *dlg = new QueryDialog(this);
	// 条件クラスのポインタ取得
	auto *cond = g_conds[condIndex];
	// ダイアログの準備
	cond->setupDialog(*dlg);
	// ダイアログが受理された場合
	if (dlg->exec() == QDialog::Accepted) {
		// 条件クラスにダイアログから得られたパラメータをロード
		cond->loadParamFromDialog(dlg->result());
		// 条件リストモデルに条件を追加
		_clm->addCondition(cond->clone());
	}
}

void MainWindow::deleteCondition() {
	// 現在選択されているインデックス取得
	const auto index = _ui->lvQueryView->currentIndex();
	// インデックスが有効なら行を削除
	if (index.isValid())
		_clm->removeRow(index.row());
}

void MainWindow::clearCondition() {
	// 条件リストモデルをクリア
	_clm->clear();
}

void MainWindow::checkCondition() {
	// ConditionListが有効なクエリを持っているならEnable
	_ui->pbQuery->setEnabled(_clm->isConditionValid());
}

void MainWindow::deleteThumbnails() {
	myTn.clearThumbnail();
}

void MainWindow::queryEdit(const QModelIndex &index) {
	// Queryの編集(Condition*)
	auto *cond = _clm->data(index, Qt::ItemDataRole::UserRole).value<Condition *>();

	// クエリダイアログの作成
	auto *dlg = new QueryDialog(this);
	// ダイアログの準備
	cond->setupDialog(*dlg);
	// ダイアログが受理された場合
	if (dlg->exec() == QDialog::Accepted) {
		// 条件クラスにダイアログから得られたパラメータをロード
		cond->loadParamFromDialog(dlg->result());
		// 条件リストモデルのデータを更新
		_clm->setData(index, QVariant::fromValue(static_cast<void *>(cond)), Qt::ItemDataRole::UserRole);
	}
}

void MainWindow::resultViewDoubleClicked(const QModelIndex &index) {
	Q_ASSERT(_ui->lvResult->model());
	// モデルからデータを取得
	const auto poseId = dg::ConvertQV<PoseId>(_ui->lvResult->model()->data(index, Qt::UserRole));
	const auto fileId = myDb_c.getFileId(poseId);
	// パスを取得
	const QString path = myDb_c.getFilePath(fileId);
	// パスが空でなければ
	if (!path.isEmpty()) {
		QDesktopServices::openUrl(QUrl::fromLocalFile(path));
	}
}

void MainWindow::detachDB() {
	// Messageboxで本当にDetachするか確認する
	if (QMessageBox::question(this, "Detach Database", "Are you sure you want to detach the database file?",
							  QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes) {
		mySet.clearValue(MySettings::Entry::DBFileName);
		// サムネイルも一緒にクリア
		myTn.clearThumbnail();
		qApp->quit();
	}
}

void MainWindow::deleteBlacklist() {
	myDb.deleteBlacklist();
}
