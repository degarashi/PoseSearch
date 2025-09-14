#include "directioncircle.hpp"
#include <QPainter>
#include <QMatrix4x4>
#include <QMouseEvent>

DirectionCircle::DirectionCircle(QWidget *parent)
	: QWidget{parent},
	  _margin(2),
	  _angleDeg(0)
{}
QSize DirectionCircle::minimumSizeHint() const {
	return {64,64};
}
QSize DirectionCircle::sizeHint() const {
	return {64,64};
}

void DirectionCircle::paintEvent(QPaintEvent* event) {
	QPen pen(Qt::black);
	pen.setWidth(2);

	QPainter painter(this);
	painter.setWindow(0, 0, 100, 100);
	painter.setRenderHint(QPainter::RenderHint::Antialiasing, true);
	painter.setPen(pen);
	painter.setBrush(Qt::white);
	painter.drawEllipse(_margin, _margin, 100-_margin*2, 100-_margin*2);

	QVector3D dir{0,0,1};
	QMatrix4x4 mRot;
	mRot.rotate(_angleDeg, {0,1,0});
	dir = mRot.map(dir);
	const float len = 50-_margin*2 - 5;
	painter.drawLine(50, 50, 50+dir.x()*len, 50-dir.z()*len);
}

void DirectionCircle::mousePressEvent(QMouseEvent* event) {
	_calcDir(event->pos());
}

void DirectionCircle::mouseMoveEvent(QMouseEvent* event) {
	_calcDir(event->pos());
}

void DirectionCircle::_calcDir(const QPoint& pos) {
	constexpr float PI = 3.141592;
	const auto s = size();
	const QPoint center(s.width()/2, s.height()/2);

	QVector2D dir(pos - center);
	dir.normalize();
	dir.setY(-dir.y());
	float ang;
	if(dir.x() >= 0) {
		if(dir.y() >= 0)
			ang = std::asin(dir.x());
		else
			ang = PI/2 + std::acos(dir.x());
	} else {
		if(dir.y() < 0)
			ang = PI + std::asin(-dir.x());
		else
			ang = PI+PI/2 + std::acos(-dir.x());
	}
	ang = qRadiansToDegrees(ang);
	setAngleDeg(ang);
}

void DirectionCircle::setAngleDeg(const int deg) {
	if(_angleDeg != deg) {
		_angleDeg = std::clamp<int>(deg, 0, 360);
		if(_angleDeg == 360)
			_angleDeg = 0;
		emit angleChanged(_angleDeg);
		repaint();
	}
}
