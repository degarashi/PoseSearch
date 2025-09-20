#pragma once
#include <QDialog>

namespace Ui {
	class PoseInfoDialog;
}

/**
 * @brief PoseInfoDialog
 *
 * ポーズ情報を表示するためのダイアログクラス。
 */
class PoseInfoDialog final : public QDialog {
		Q_OBJECT

	public:
		explicit PoseInfoDialog(int poseId, QWidget *parent = nullptr);

	private:
		QSharedPointer<Ui::PoseInfoDialog> _ui;
};
