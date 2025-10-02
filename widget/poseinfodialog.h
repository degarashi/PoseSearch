#pragma once
#include <QDialog>
#include "id.hpp"

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
		explicit PoseInfoDialog(PoseId poseId, QWidget *parent = nullptr);

	private:
		QSharedPointer<Ui::PoseInfoDialog> _ui;
};
