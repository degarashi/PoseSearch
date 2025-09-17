#include "directionparam_pitch.h"
#include "aux_f/math.hpp"

DirectionParamPitch::DirectionParamPitch(QWidget *parent) : DirectionParamPitch(0, parent) {
}
DirectionParamPitch::DirectionParamPitch(const int initial, QWidget *parent) : DirectionParam2D(initial, parent) {
	initManipulator(true);
}
