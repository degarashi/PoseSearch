#pragma once
#include <QWidget>

// Pitch設定用Widget
// 0=真横 90=真上 -90=真下
class DirectionArc : public QWidget {
	Q_OBJECT
	public:
		explicit DirectionArc(QWidget *parent = nullptr);
		QSize minimumSizeHint() const override;
		QSize sizeHint() const override;

	protected:
		void paintEvent(QPaintEvent* event) override;
		void mousePressEvent(QMouseEvent* event) override;
		void mouseMoveEvent(QMouseEvent* event) override;

	private:
		int			_margin;
		int			_angleDeg;

		/**
		 * @brief 中心から座標pへの角度計算
		 * @param p マウスカーソルの位置
		 */
		void		_calcDirAndApply(const QPoint& p);

	public slots:
		/**
		 * @brief 角度の設定
		 * @param deg 設定する角度（度）
		 */
		void setAngleDeg(int deg);
	signals:
		/**
		 * @brief 角度変更シグナル
		 * @param deg 変更後の角度（度）
		 */
		void angleChanged(int deg);
};
