#include "imageview.h"

ImageView::ImageView(QWidget *parent) : QWidget(parent) {
}

void ImageView::paintEvent(QPaintEvent *) {
	QPainter painter(this);
	painter.drawImage(rect(), image);
}

void ImageView::setImage(const QImage &img) {
	image = img;
	update();
}
