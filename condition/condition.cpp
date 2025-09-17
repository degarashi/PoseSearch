#include "condition.hpp"
#include <QJsonDocument>
#include "param/float_slider_param.h"
#include "param/paramwrapper.h"
#include "param/querydialog.h"

namespace {
	// スライダーの範囲を定義
	constexpr dg::Range<float> SliderRange{-2.f, 2.f};
} // namespace

// --- Condtiion ---
void Condition::_clone(Condition &dst) const {
	dst._ratio = _ratio;
}

QString Condition::_textPresent(const QString &src) const {
	return QString("%1 {ratio=%2, %3}").arg(dialogName()).arg(_ratio).arg(src);
}

void Condition::setupDialog(QueryDialog &dlg) const {
	// Ratio項
	dlg.addParam(new ParamWrapper(new FloatSliderParam(SliderRange, _ratio), "Ratio"));
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
CEREAL_REGISTER_TYPE(Cond_Tag)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Condition, Cond_BodyDir)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Condition, Cond_Tag)
CEREAL_REGISTER_DYNAMIC_INIT(Cond_BodyDir)
CEREAL_REGISTER_DYNAMIC_INIT(Cond_Tag)
