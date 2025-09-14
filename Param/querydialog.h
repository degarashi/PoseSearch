#pragma once
#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui {
	class QueryDialog;
}
QT_END_NAMESPACE

// 姿勢検索に使用する条件に対し、パラメータをユーザーが設定する為のダイアログ
class QueryDialog : public QDialog {
		Q_OBJECT

	public:
		/**
		 * @brief QueryDialogクラスのコンストラクタ
		 *
		 * UIをセットアップし、ダイアログのサイズを固定
		 *
		 * @param parent 親ウィジェットへのポインタ
		 */
		explicit QueryDialog(QWidget *parent = nullptr);

		/**
		 * @brief パラメータウィジェットをダイアログに追加
		 *
		 * 指定されたパラメータウィジェットと区切り線を追加
		 *
		 * @param param 追加するパラメータウィジェット
		 */
		void addParam(QWidget *param);

		/**
		 * @brief 全てのパラメータの結果を取得
		 *
		 * ダイアログ内の全てのパラメータの値をリストとして返す
		 *
		 * @return パラメータの値のリスト
		 */
		QVariantList result() const;

	private:
		std::shared_ptr<Ui::QueryDialog> _ui;
};
