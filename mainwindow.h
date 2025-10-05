#pragma once
#include <QMainWindow>
#include <QSqlDatabase>

QT_BEGIN_NAMESPACE
namespace Ui {
	class MainWindow;
}
QT_END_NAMESPACE

class ResultPathModel;
class ConditionModel;
class MainWindow : public QMainWindow {
		Q_OBJECT

	public:
		MainWindow(QWidget *parent = nullptr);

	private:
		using Cond_SP = std::shared_ptr<ConditionModel>;
		Cond_SP _clm;
		ResultPathModel *_rpm;
		QSharedPointer<Ui::MainWindow> _ui;

		void _setConditionModel(Cond_SP clm);

	private slots:
		void query();
		void addCondition();
		void deleteCondition();
		void clearCondition();
		void checkCondition();
		void deleteThumbnails();
		void queryEdit(const QModelIndex &index);
		void detachDB();
		void loadConditions();
		void saveConditions();
		void deleteBlacklist();

		void resultViewDoubleClicked(const QModelIndex &index);
};
