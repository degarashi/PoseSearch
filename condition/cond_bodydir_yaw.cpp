#include <qmath.h>
#include "aux_f_q/convert.hpp"
#include "condition.hpp"
#include "param/directionparam_yaw.h"
#include "param/paramwrapper.h"
#include "param/querydialog.h"

// ------------ Cond_BodyDirYaw ----------------------
Cond_BodyDirYaw::Cond_BodyDirYaw() : _yawDir(-1.f, 0.f) {
}

QString Cond_BodyDirYaw::dialogName() const {
	return "Body Direction (Yaw)";
}

Condition_SP Cond_BodyDirYaw::clone() const {
	auto ret = std::make_shared<Cond_BodyDirYaw>();
	Condition::_clone(*ret);
	ret->_yawDir = _yawDir;
	return ret;
}

QString Cond_BodyDirYaw::textPresent() const {
	return _textPresent(QString("yaw-dir: %1").arg(dg::VecToString(_yawDir)));
}

void Cond_BodyDirYaw::setupDialog(QueryDialog &dlg) const {
	dlg.addParam(new ParamWrapper(new DirectionParamYaw(_yawDir), "Yaw-Direction"));
	Condition::setupDialog(dlg);
}

void Cond_BodyDirYaw::loadParamFromDialog(const QVariantList &vl) {
	bool ok;
	const int yaw_deg = vl[0].toInt(&ok);
	Q_ASSERT(ok);

	const float yaw_rad = qDegreesToRadians(yaw_deg);
	_yawDir = {std::sin(yaw_rad), std::cos(yaw_rad)};
	Condition::loadParamFromDialog(vl);
}

QuerySeed Cond_BodyDirYaw::getSqlQuery(const QueryParam &param) const {
	return {
		QString("WITH %1 AS ( "
				"SELECT poseId, (2.0 - distance)/2 AS score "
				"	FROM MasseTorsoVec "
				"	WHERE yaw MATCH :body_yaw "
				"	LIMIT :limit ) ")
			.arg(param.outputTableName),
		{
			{":body_yaw", dg::VecToByteArray(param.ratio < 0.f ? -_yawDir : _yawDir)},
		},
		std::abs(param.ratio),
	};
}
