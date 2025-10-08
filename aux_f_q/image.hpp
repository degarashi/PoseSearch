#pragma once
#include <QImage>
#include <QImageIOHandler>

namespace dg {
	// Exif の回転情報を反映
	QImage RotateByExif(const QImageIOHandler::Transformations tfFlag, const QImage &img);
} // namespace dg
