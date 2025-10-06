#include <QtMath>
#include "aux_f_q/convert.hpp"
#include "aux_f_q/q_value.hpp"
#include "condition.hpp"
#include "param/float_slider_param.h"
#include "param/paramwrapper.h"
#include "param/querydialog.h"

namespace {
	constexpr dg::Degree FLEX_MIN{-90.f};
	constexpr dg::Degree FLEX_MAX{180.f};
	constexpr std::array<const char *, 2> FLEX_LABELS{"Flexion Left", "Flexion Right"};

	constexpr const char *SQL_TEMPLATE = R"(
		WITH %1 AS (
			SELECT
				poseId,
				SUM(
					2.0 - (
						POW(ABS(angleRad - :target_left) * (1 - is_right), 2) +
						POW(ABS(angleRad - :target_right) * is_right, 2)
					) / 2
				) AS score
			FROM ThighFlexion
			GROUP BY poseId
			LIMIT :limit
		)
	)";
} // namespace

// ------------ Cond_ThighFlexion ----------------------
QString Cond_ThighFlexion::dialogName() const {
	return QStringLiteral("Thigh Flexion");
}

Condition_SP Cond_ThighFlexion::clone() const {
	auto ret = std::make_shared<Cond_ThighFlexion>();
	Condition::_clone(*ret);
	ret->_flexDeg = _flexDeg;
	return ret;
}

QString Cond_ThighFlexion::textPresent() const {
	return QStringLiteral("thigh-flexion-dir: %1, %2").arg(_flexDeg[0].toString(), _flexDeg[1].toString());
}

void Cond_ThighFlexion::setupDialog(QueryDialog &dlg) const {
	for (int i = 0; i < 2; ++i) {
		dlg.addParam(new ParamWrapper(new FloatSliderParam({FLEX_MIN.get(), FLEX_MAX.get()}, _flexDeg[i].get()),
									  FLEX_LABELS[i]));
	}
	Condition::setupDialog(dlg);
}
bool Cond_ThighFlexion::_supportNegativeRatio() const {
	return false;
}

void Cond_ThighFlexion::loadParamFromDialog(const QVariantList &vl) {
	for (int i = 0; i < 2 && i < vl.size(); ++i) {
		_flexDeg[i].set(dg::ConvertQV<float>(vl[i]));
	}
	Condition::loadParamFromDialog(vl);
}
namespace {
	// インジェクション対策(識別子のバリデーション)
	void ValidateTableName(const QString &name) {
		static QRegularExpression s_reg(R"([^A-Za-z0-9_])");
		if (name.contains(s_reg)) {
			throw dg::InvalidInput("Invalid table name");
		}
	}
} // namespace
QuerySeed Cond_ThighFlexion::getSqlQuery(const QueryParam &param) const {
	ValidateTableName(param.outputTableName);
	const QString sql = QString(SQL_TEMPLATE).arg(param.outputTableName);
	Q_ASSERT(param.ratio >= 0.f);
	return {
		sql,
		{
			{":target_left", _flexDeg[0].toRadian().get()},
			{":target_right", _flexDeg[1].toRadian().get()},
		},
		param.ratio,
	};
}
