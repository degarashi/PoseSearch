#include <QVector3D>

namespace dg {
	QVector3D YawPitchToVec(float yawDeg, float pitchDeg);
	std::pair<float, float> VecToYawPitch(const QVector3D &vec);

	template <typename T>
	auto Square(const T &t) {
		return t * t;
	}
}
