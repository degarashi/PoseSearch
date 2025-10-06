#pragma once
#include <QVariant>
#include "aux_f/value.hpp"

struct CondParam {
		float current;
		dg::FRange range;
};
Q_DECLARE_METATYPE(CondParam);
