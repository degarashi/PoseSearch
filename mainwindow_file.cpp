#include <QFileDialog>
#include <QMessageBox>
#include <cereal/archives/binary.hpp>
#include <cereal/cereal.hpp>
#include <cereal/types/memory.hpp>
#include <fstream>
#include "mainwindow.h"
#include "widget/conditionmodel.hpp"
#include "condition/condition.hpp"

void MainWindow::loadConditions() {
	// ファイルダイアログでファイルを選択
	QString fileName = QFileDialog::getOpenFileName(this, tr("Open Conditions"), "", tr("Condition Files (*.cond)"));
	if (fileName.isEmpty())
		return;

	std::ifstream ifs(fileName.toStdString());
	if (!ifs.is_open()) {
		QMessageBox::critical(this, "Error", "Failed to open file for loading conditions.");
		return;
	}
	cereal::BinaryInputArchive archive(ifs);
	Cond_SP clm;
	try {
		archive(clm);
	}
	catch (const cereal::Exception &e) {
		QMessageBox::critical(this, "Error", QString("Failed to load conditions: %1").arg(e.what()));
		return;
	}
	_setConditionModel(clm); // ロードしたモデルでUIを更新
}

void MainWindow::saveConditions() {
	// ファイルダイアログでファイルを選択
	QString fileName = QFileDialog::getSaveFileName(this, tr("Save Conditions"), "", tr("Condition Files (*.cond)"));
	if (fileName.isEmpty())
		return;

	std::ofstream os(fileName.toStdString());
	if (!os.is_open()) {
		QMessageBox::critical(this, "Error", "Failed to open file for saving conditions.");
		return;
	}
	cereal::BinaryOutputArchive archive(os);
	try {
		archive(_clm);
	}
	catch (const cereal::Exception &e) {
		QMessageBox::critical(this, "Error", QString("Failed to save conditions: %1").arg(e.what()));
		return;
	}
}
