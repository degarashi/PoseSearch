#include "spin_intparam.h"
#include "ui_spin_intparam.h"

SpinIntParam::SpinIntParam(QWidget *parent)
	: QWidget{parent},
	  _ui(new Ui::SpinIntParam)
{
	_ui->setupUi(this);
	connect(_ui->sbParam, &QSpinBox::valueChanged, this, &SpinIntParam::valueChanged);
}

void SpinIntParam::setName(const QString& name) {
	_ui->lbParam->setText(name);
}

int SpinIntParam::value() const {
	return _ui->sbParam->value();
}

void SpinIntParam::setRange(int min, int max) {
	_ui->sbParam->setRange(min, max);
}

void SpinIntParam::setValue(int val) {
	_ui->sbParam->setValue(val);
}
