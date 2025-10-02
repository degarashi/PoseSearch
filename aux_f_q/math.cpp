#include "math.hpp"
#include <QMatrix4x4>
#include <cmath>

namespace dg {
	QVector3D YawPitchToVec(const float yawDeg, const float pitchDeg) {
		// X軸は右
		// Y軸は上
		// Z軸は奥行き+
		// QMatrix.rotateはDegree指定
		QMatrix4x4 m;
		m.rotate(yawDeg, {0, 1, 0});
		m.rotate(-pitchDeg, {1, 0, 0});

		QVector3D dir{0, 0, 1};
		dir = m.map(dir);
		return dir;
	}
	std::pair<float, float> VecToYawPitch(const QVector3D &vec) {
		// Yawの計算
		float yawDeg = qRadiansToDegrees(atan2(vec.x(), vec.z()));
		// この時点では[-180, 180]の範囲
		if(yawDeg < 0)
			yawDeg += 360;

		// Pitchの計算
		// ベクトルをXZ平面に射影したベクトルの長さ
		const float xz_len = sqrt(Square(vec.x()) + Square(vec.z()));
		// Pitch = この射影ベクトルと元のベクトルとの間の角度
		float pitchDeg;
		// ゼロまたはゼロに近い値での除算を避ける
		if (xz_len > 1e-6) {
			// Y-XZ平面での角度
			pitchDeg = qRadiansToDegrees(atan2(vec.y(), xz_len));
		}
		else {
			// ベクトルがY軸に沿っている場合
			if (vec.y() > 0) {
				pitchDeg = 90.0f;
			}
			else if (vec.y() < 0) {
				pitchDeg = -90.0f;
			}
			else {
				// ゼロベクトルの場合
				pitchDeg = 0.0f;
			}
		}
		// Pitchを[-90, 90]の範囲に正規化
		pitchDeg = std::clamp<float>(pitchDeg, -90, 90);

		return {yawDeg, pitchDeg};
	}
} // namespace dg
