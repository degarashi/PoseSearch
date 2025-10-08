#include "image.hpp"

namespace dg {
	QImage RotateByExif(const QImageIOHandler::Transformations tfFlag, const QImage &img) {
		using enum QImageIOHandler::Transformation;
		// 必要に応じて回転・反転を適用
		switch (tfFlag) {
			case 0:
				return img;
			case TransformationRotate90:
				return img.transformed(QTransform().rotate(90));
			case TransformationRotate180:
				return img.transformed(QTransform().rotate(180));
			case TransformationRotate270:
				return img.transformed(QTransform().rotate(270));
			case TransformationMirror: {
				QTransform t;
				t.scale(-1, 1); // 水平方向に反転
				return img.transformed(t);
			}
			default:
				qWarning() << "not supported transform mode-type";
		}
		return {};
	}
} // namespace dg
