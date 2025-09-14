#include "directionparam2d.h"
#include <QTimer>
#include "ui_directionparam_2d.h"
#include "widget/directionarc.hpp"
#include "widget/directioncircle.hpp"

DirectionParam2D::DirectionParam2D(QWidget *parent) : DirectionParam2D(0, parent) {
}
DirectionParam2D::DirectionParam2D(const int initial, QWidget *parent) :
	ParamBaseT<int>(parent), _ui(new Ui::DirectionParam2D) {
	_ui->setupUi(this);

	// 初期値の設定（非同期実行）
	QTimer::singleShot(0, this, [this, initial] { setValue(initial); });
}

void DirectionParam2D::initManipulator(const bool bPitch) {
	QWidget *w;
	if (bPitch) {
		auto *arc = new DirectionArc(this);
		_ui->body->setRange(-90, 90);
		connect(arc, &DirectionArc::angleChanged, _ui->body, &SpinIntParam::setValue);
		connect(_ui->body, &SpinIntParam::valueChanged, arc, &DirectionArc::setAngleDeg);
		w = arc;
	}
	else {
		auto *cir = new DirectionCircle(this);
		_ui->body->setRange(0, 359);
		connect(cir, &DirectionCircle::angleChanged, _ui->body, &SpinIntParam::setValue);
		connect(_ui->body, &SpinIntParam::valueChanged, cir, &DirectionCircle::setAngleDeg);
		w = cir;
	}
	_ui->frame->layout()->addWidget(w);
}

void DirectionParam2D::setName(const QString &name) {
	_ui->body->setName(name);
}

int DirectionParam2D::value() const {
	return _ui->body->value();
}

void DirectionParam2D::setValue(const int &val) {
	_ui->body->setValue(val);
}
