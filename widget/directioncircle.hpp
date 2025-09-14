#pragma once
#include <QWidget>

// 角度指定用Widget
class DirectionCircle : public QWidget {
	Q_OBJECT

	public:
		explicit DirectionCircle(QWidget *parent = nullptr);
		QSize sizeHint() const override;
		QSize minimumSizeHint() const override;
	protected:
		void paintEvent(QPaintEvent* event) override;
		void mousePressEvent(QMouseEvent* event) override;
		void mouseMoveEvent(QMouseEvent* event) override;

	private:
		int			_margin;
		int			_angleDeg;
		void _calcDir(const QPoint& p);

	public slots:
		void setAngleDeg(int deg);
	signals:
		void angleChanged(int deg);

};
