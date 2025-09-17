#pragma once
#include "directionparam2d.h"

class DirectionParamYaw : public DirectionParam2D {
		Q_OBJECT
	public:
		explicit DirectionParamYaw(QWidget *parent = nullptr);
		explicit DirectionParamYaw(const QVector2D &initial, QWidget *parent = nullptr);
};
