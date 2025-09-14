#include "directionarc.hpp"
#include <QPen>
#include <QPainter>
#include <QMouseEvent>

DirectionArc::DirectionArc(QWidget *parent)
	: QWidget{parent},
	_margin(2),
	  _angleDeg(0)
{}

QSize DirectionArc::minimumSizeHint() const {
	return {64,64};
}

QSize DirectionArc::sizeHint() const {
	return {64,64};
}

void DirectionArc::paintEvent(QPaintEvent* event) {
	QPen pen(Qt::black);
	pen.setWidth(2);

	QPainter painter(this);
	painter.setWindow(0, 0, 100, 100);
	painter.setRenderHint(QPainter::RenderHint::Antialiasing, true);

	// 外周を描画
	painter.setBrush(Qt::white);
	painter.setPen(Qt::NoPen);
	painter.drawEllipse(_margin, _margin, 100-_margin*2, 100-_margin*2);

	// 左半分を斜め線で塗りつぶし
	painter.fillRect(0,0,50,100, Qt::BrushStyle::BDiagPattern);

	// 円弧を描画
	painter.setPen(pen);
	painter.drawArc(0+_margin, _margin, 100-_margin*2, 100-_margin*2, -90*16, 180*16);

	// 現在の角度を線分で表示
	const auto rad = qDegreesToRadians(_angleDeg);
	const auto s = std::sin(rad);
	const auto c = std::cos(rad);
	const float len = 50 - _margin*2 - 5;
	painter.drawLine(50, 50, 50+c*len, 50-s*len);
}

void DirectionArc::mousePressEvent(QMouseEvent* event) {
	_calcDirAndApply(event->pos());
}
void DirectionArc::mouseMoveEvent(QMouseEvent* event) {
	_calcDirAndApply(event->pos());
}

void DirectionArc::_calcDirAndApply(const QPoint& p) {
	constexpr float PI = 3.141592;
	// ウィジェットのサイズを取得
	const auto s = size();
	// 中心座標を計算
	const QPoint center(s.width()/2, s.height()/2);

	QVector2D dir(p - center);
	dir.normalize();
	dir.setY(-dir.y());
	if(dir.x() < 0)
		return;

	// Y成分から角度を計算
	const float ang = qRadiansToDegrees(std::asin(dir.y()));
	setAngleDeg(ang);
}
void DirectionArc::setAngleDeg(int deg) {
	deg = std::clamp<int>(deg, -90, 90);
	if(_angleDeg != deg) {
		_angleDeg = deg;
		emit angleChanged(_angleDeg);
		repaint();
	}
}
