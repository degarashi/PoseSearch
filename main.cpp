#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include "aux_f/exception.hpp"
#include "mainwindow.h"
#include "singleton/my_db.hpp"
#include "singleton/my_settings.hpp"
#include "singleton/my_thumbnail.hpp"

int main(int argc, char *argv[]) {
	QApplication a(argc, argv);
	MySettings::InitializeUsing("./settings.ini");
	// データベースファイル名の取得。設定に保存されていればそれを、なければファイルダイアログを開く
	QString dbFileName = mySet_c.getValue(MySettings::Entry::DBFileName).toString();
	if (dbFileName.isEmpty()) {
		dbFileName =
			QFileDialog::getOpenFileName(nullptr, "open database...", "", "SQLite Database (*.db *.sqlite *.sqlite3)");
		if (dbFileName.isEmpty())
			return 0;
		// 選択されたファイル名を設定に保存
		mySet.setValue(MySettings::Entry::DBFileName, dbFileName);
	}

	try {
		auto db = std::make_unique<dg::sql::Database>("DGDB", dbFileName, dg::sql::FeatureV{},
													  dg::sql::PragmaV{{"foreign_keys", "true"}});
		dg::LoadVecExtension(*db);
		MyDatabase::InitializeUsing(std::move(db));
		MyThumbnail::InitializeUsing();
		auto dInit = MyDatabase::ScopeDeinit();
		auto dInitT = MyThumbnail::ScopeDeinit();

		MainWindow mw(nullptr);
		mw.show();
		return a.exec();
	}
	catch (const dg::RuntimeError &e) {
		QMessageBox::critical(nullptr, "Database Error", QString::fromStdString(e.s_what()));
	}
	return 1;
}
