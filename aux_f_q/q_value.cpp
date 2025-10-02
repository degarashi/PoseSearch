#include "q_value.hpp"

namespace dg::detail {
	bool _ConvertQV(const QVariant &v, bool *) {
		return v.toBool();
	}
	float _ConvertQV(const QVariant &v, float *) {
		return v.toFloat();
	}
	QByteArray _ConvertQV(const QVariant &v, QByteArray *) {
		return v.toByteArray();
	}
	QString _ConvertQV(const QVariant &v, QString *) {
		return v.toString();
	}
} // namespace dg::detail
