#include "directionparam_yaw.h"
#include <QVector2D>
#include "aux_f_q/math.hpp"

DirectionParamYaw::DirectionParamYaw(QWidget *parent) : DirectionParamYaw({0, 1}, parent) {
}
DirectionParamYaw::DirectionParamYaw(const QVector2D &initial, QWidget *parent) :
	DirectionParam2D(dg::VecToYawPitch(QVector3D(initial.x(), 0, initial.y())).first, parent) {
	initManipulator(false);
}
