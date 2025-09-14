#include "../condition.hpp"
#include "aux_f/convert.hpp"
#include "param/directionparam3d.h"
#include "param/paramwrapper.h"
#include "param/querydialog.h"

// ------------ Cond_BodyDir ----------------------
Cond_BodyDir::Cond_BodyDir() : _dir(1.f, 0.f, 0.f) {
}

QString Cond_BodyDir::dialogName() const {
	return "Body Direction";
}

Condition_SP Cond_BodyDir::clone() const {
	auto ret = std::make_shared<Cond_BodyDir>();
	Condition::_clone(*ret);
	ret->_dir = _dir;
	return ret;
}

QString Cond_BodyDir::textPresent() const {
	return _textPresent(QString("dir: %1").arg(VecToString(_dir)));
}

void Cond_BodyDir::setupDialog(QueryDialog &dlg) const {
	dlg.addParam(new ParamWrapper(new DirectionParam3D(_dir), "Direction"));
	Condition::setupDialog(dlg);
}

void Cond_BodyDir::loadParamFromDialog(const QVariantList &vl) {
	_dir = vl[0].value<QVector3D>();
	Condition::loadParamFromDialog(vl);
}

QuerySeed Cond_BodyDir::getSqlQuery(const QueryParam &param) const {
	const auto ba = dg::VecToByteArray(param.ratio < 0.f ? -_dir : _dir);
	return {
		QString("WITH tmp AS (SELECT poseId, distance "
				"FROM MasseTorsoVec "
				"WHERE torsoDir MATCH :body_dir "
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
