#include "directionarc.hpp"
#include <QMouseEvent>
#include <QPainter>
#include <QPen>
#include <QVector2D>
#include <cmath>

namespace {
	// 描画用の論理サイズ（paintEventでスケーリング）
	constexpr int LogicalSize = 100;
	constexpr int DefaultHint = 64;
	constexpr int ArcSpanDeg = 180;
	constexpr int ArcStartDeg = -90;
	constexpr int PenWidth = 2;
	constexpr int InnerPadding = 5;
	constexpr int DefaultMargin = 2;
	constexpr int MinDeg = -90;
	constexpr int MaxDeg = 90;
} // namespace

DirectionArc::DirectionArc(QWidget *parent) : QWidget{parent}, _margin(DefaultMargin), _angleDeg(0) {
}

QSize DirectionArc::minimumSizeHint() const {
	return {DefaultHint, DefaultHint};
}

QSize DirectionArc::sizeHint() const {
	return minimumSizeHint();
}

void DirectionArc::_drawOuterCircle(QPainter &painter) const {
	painter.setBrush(Qt::white);
	painter.setPen(Qt::NoPen);
	painter.drawEllipse(_margin, _margin, LogicalSize - _margin * 2, LogicalSize - _margin * 2);
}

void DirectionArc::_drawLeftHalfPattern(QPainter &painter) const {
	painter.fillRect(0, 0, LogicalSize / 2, LogicalSize, Qt::BDiagPattern);
}

void DirectionArc::_drawArc(QPainter &painter) const {
	QPen pen(Qt::black, PenWidth);
	painter.setPen(pen);
	painter.drawArc(_margin, _margin, LogicalSize - _margin * 2, LogicalSize - _margin * 2, ArcStartDeg * 16,
					ArcSpanDeg * 16);
}

void DirectionArc::_drawAngleLine(QPainter &painter) const {
	const auto rad = qDegreesToRadians(_angleDeg);
	const float sinVal = std::sin(rad);
	const float cosVal = std::cos(rad);
	const float len = (LogicalSize / 2.f) - _margin * 2.f - InnerPadding;

	QPen pen(Qt::black, PenWidth);
	painter.setPen(pen);
	painter.drawLine(LogicalSize / 2.f, LogicalSize / 2.f, LogicalSize / 2.f + cosVal * len,
					 LogicalSize / 2.f - sinVal * len);
}

void DirectionArc::paintEvent(QPaintEvent *event) {
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing, true);

	// 実サイズに合わせてスケーリング
	painter.setWindow(0, 0, LogicalSize, LogicalSize);
	painter.setViewport(rect());

	_drawOuterCircle(painter);
	_drawLeftHalfPattern(painter);
	_drawArc(painter);
	_drawAngleLine(painter);
}

void DirectionArc::mousePressEvent(QMouseEvent *event) {
	_handleMouseInput(event->pos());
}

void DirectionArc::mouseMoveEvent(QMouseEvent *event) {
	_handleMouseInput(event->pos());
}

void DirectionArc::_handleMouseInput(const QPoint &p) {
	const QPoint center(width() / 2, height() / 2);

	QVector2D dir(p - center);
	if (dir.lengthSquared() == 0.0f)
		return;

	dir.normalize();
	dir.setY(-dir.y());

	float ang;
	if (dir.x() < 0) {
		if (dir.y() < 0)
			ang = MinDeg;
		else
			ang = MaxDeg;
	}
	else
		ang = qRadiansToDegrees(std::asin(dir.y()));
	setAngleDeg(static_cast<int>(ang));
}

void DirectionArc::setAngleDeg(int deg) {
	deg = std::clamp(deg, MinDeg, MaxDeg);
	if (_angleDeg != deg) {
		_angleDeg = deg;
		emit angleChanged(_angleDeg);
		update();
	}
}
