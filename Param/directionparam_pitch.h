#pragma once
#include "directionparam2d.h"

class DirectionParamPitch : public DirectionParam2D {
		Q_OBJECT
	public:
		explicit DirectionParamPitch(QWidget *parent = nullptr);
		explicit DirectionParamPitch(int initial, QWidget *parent = nullptr);
};
