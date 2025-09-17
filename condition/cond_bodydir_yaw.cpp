#include <qmath.h>
#include "aux_f/convert.hpp"
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
	const auto ba = dg::VecToByteArray(param.ratio < 0.f ? -_yawDir : _yawDir);
	return {
		QString("WITH tmp AS (SELECT poseId, distance "
				"FROM MasseTorsoVec "
				"WHERE yaw MATCH :body_dir "
				"LIMIT %1 ), %2 AS ( "
				"SELECT tmp.poseId, (2.0 - tmp.distance)/2 AS score "
				"FROM tmp "
				"INNER JOIN MasseTorsoDir AS MT "
				"	ON tmp.poseId = MT.poseId "
				") ")
			.arg(param.limit)
			.arg(param.outputTableName),
		{
			{":body_dir", std::move(ba)},
		},
		std::abs(param.ratio),
	};
}
