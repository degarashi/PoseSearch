#include "condition.hpp"
#include <QJsonDocument>
#include <QSqlQuery>
#include <QSqlRecord>
#include "aux_f_q/sql/database.hpp"
#include "param/float_slider_param.h"
#include "param/paramwrapper.h"
#include "param/querydialog.h"

// --- QuerySeed ---
QSqlQuery QuerySeed::exec(dg::sql::Database &db, const QString &qtext, const int limit) const {
	QSqlQuery q(db.database());
	q.prepare(queryText + qtext);
	setupParams(q, limit);
	dg::sql::Query(q);
	return std::move(q);
}
void QuerySeed::setupParams(QSqlQuery &q, const int limit) const {
	for (auto &p : queryParams)
		q.bindValue(p.first, p.second);
	q.bindValue(":ratio", ratio);
	q.bindValue(":limit", limit);
}

namespace {
	// Ratioスライダーのデフォルト範囲
	constexpr float SliderRange = 2.f;
	constexpr dg::FRange DefaultRange{0.f, SliderRange};
	constexpr dg::FRange DefaultRangeN{-SliderRange, SliderRange};
} // namespace

bool Condition::_supportNegativeRatio() const {
	return true;
}

// --- Condtiion ---
void Condition::_clone(Condition &dst) const {
	dst._ratio = _ratio;
}

void Condition::setupDialog(QueryDialog &dlg) const {
	// Ratio項
	dlg.addParam(new ParamWrapper(new FloatSliderParam(getRatioRange(), _ratio), "Ratio"));
	dlg.setWindowTitle(dialogName());
}

void Condition::loadParamFromDialog(const QVariantList &vl) {
	if (vl.isEmpty())
		throw dg::InvalidInput("Invalid parameter list received from dialog");
	_ratio = vl.back().toFloat();
}

float Condition::getRatio() const noexcept {
	return _ratio;
}
void Condition::setRatio(const float r) noexcept {
	_ratio = r;
}
dg::FRange Condition::getRatioRange() const noexcept {
	return {_supportNegativeRatio() ? -SliderRange : 0.f, SliderRange};
}
// -----------------------------------------
QJsonArray VecToJArray(const QVector3D &v) {
	return {v.x(), v.y(), v.z()};
}

QString AttachGUID(QJsonObject &js) {
	const auto uid = QUuid::createUuid();
	const QString guid = uid.toString(QUuid::StringFormat::Id128);
	js["id"] = guid;
	return guid;
}

#include <cereal/archives/binary.hpp>
CEREAL_REGISTER_TYPE(Cond_BodyDir)
CEREAL_REGISTER_TYPE(Cond_BodyDirYaw)
CEREAL_REGISTER_TYPE(Cond_BodyDirPitch)
CEREAL_REGISTER_TYPE(Cond_Tag)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Condition, Cond_BodyDir)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Condition, Cond_BodyDirYaw)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Condition, Cond_BodyDirPitch)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Condition, Cond_Tag)
CEREAL_REGISTER_DYNAMIC_INIT(Cond_BodyDir)
CEREAL_REGISTER_DYNAMIC_INIT(Cond_BodyDirYaw)
CEREAL_REGISTER_DYNAMIC_INIT(Cond_BodyDirPitch)
CEREAL_REGISTER_DYNAMIC_INIT(Cond_Tag)
