#include "imageview.h"
#include <QPainter>

ImageView::ImageView(QWidget *parent) : QWidget(parent) {
}

void ImageView::paintEvent(QPaintEvent *) {
	QPainter painter(this);
	// 空画像の場合は描画しない
	if (!image.isNull()) {
		painter.drawImage(rect(), image);
	}
}

void ImageView::setImage(const QImage &img) {
	if (!img.isNull()) {
		// ウィジェットサイズに合わせてスケーリングして保持
		image = img.scaled(size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
	}
	else {
		image = QImage();
	}
	update();
}
