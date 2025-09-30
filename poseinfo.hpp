#pragma once

#include <QVector2D>
#include <QVector3D>
#include <vector>
#include "aux_f/angle.hpp"

struct PoseInfo {
		std::vector<QVector2D> landmarks;
		QString torsoMethod;
		QVector3D torsoDir;
		// [0] = left, [1] = right
		QVector3D thighDir[2];
		QVector3D crusDir[2];
		dg::Degree thighFlex[2];
		dg::Degree crusFlex[2];
		QRectF rect;
};
