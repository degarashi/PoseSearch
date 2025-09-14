#pragma once
#include <QWidget>

namespace Ui { class SpinIntParam; }

class SpinIntParam : public QWidget {
	Q_OBJECT
	public:
		/**
		 * @brief コンストラクタ
		 * @param parent 親ウィジェット
		 */
		explicit SpinIntParam(QWidget *parent = nullptr);
		std::shared_ptr<Ui::SpinIntParam>	_ui;
		/**
		 * @brief パラメータ名を設定
		 * @param name パラメータ名
		 */
		void setName(const QString& name);
		/**
		 * @brief 現在の値を取得
		 * @return 現在の値
		 */
		int value() const;
		/**
		 * @brief 値の範囲を設定
		 * @param min 最小値
		 * @param max 最大値
		 */
		void setRange(int min, int max);

	public slots:
		/**
		 * @brief 値を設定
		 * @param val 設定する値
		 */
		void setValue(int val);

	signals:
		/**
		 * @brief 値が変更されたことを通知
		 * @param val 変更後の値
		 */
		void valueChanged(int val);
};
