#include <qmath.h>
#include "aux_f_q/convert.hpp"
#include "aux_f/value.hpp"
#include "condition.hpp"
#include "param/directionparam_pitch.h"
#include "param/paramwrapper.h"
#include "param/querydialog.h"

// ------------ Cond_BodyDirPitch ----------------------
Cond_BodyDirPitch::Cond_BodyDirPitch() : _pitch(0) {
}

QString Cond_BodyDirPitch::dialogName() const {
	return "Body Direction (Pitch)";
}

Condition_SP Cond_BodyDirPitch::clone() const {
	auto ret = std::make_shared<Cond_BodyDirPitch>();
	Condition::_clone(*ret);
	ret->_pitch = _pitch;
	return ret;
}

QString Cond_BodyDirPitch::textPresent() const {
	return QString("pitch: %1").arg(_pitch);
}

void Cond_BodyDirPitch::setupDialog(QueryDialog &dlg) const {
	dlg.addParam(new ParamWrapper(new DirectionParamPitch(_pitch), "Pitch"));
	Condition::setupDialog(dlg);
}

bool Cond_BodyDirPitch::_supportNegativeRatio() const {
	return false;
}

void Cond_BodyDirPitch::loadParamFromDialog(const QVariantList &vl) {
	bool ok;
	_pitch = vl[0].toInt(&ok);
	Q_ASSERT(ok);
	Condition::loadParamFromDialog(vl);
}

QuerySeed Cond_BodyDirPitch::getSqlQuery(const QueryParam &param) const {
	// _pitchは[-90, 90]の範囲
	// テーブルに格納してあるのは[-1, 1]
	const auto clampedVal = dg::Remap(static_cast<float>(_pitch), -90.f, 90.f, -1.f, 1.f);
	auto ba = dg::VecToByteArray(clampedVal);
	return {
		QString(
			// Vec0から値を取り出す -> outputTable
			"WITH %1 AS ( "
			"SELECT poseId, (2.0 - distance)/2 AS score "
			"FROM MasseTorsoVec "
			"WHERE pitch MATCH :pitch_val "
			"LIMIT :limit ) ")
			.arg(param.outputTableName),
		{
			{":pitch_val", std::move(ba)},
		},
		param.ratio,
	};
}
