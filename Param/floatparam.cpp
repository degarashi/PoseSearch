#include "floatparam.h"
#include "ui_float_param.h"

FloatParam::FloatParam(const float initial, QWidget *parent) :
	ParamBaseT(parent), _ui(std::make_shared<Ui::FloatParam>()) {
	_ui->setupUi(this);

	setValue(initial);
}

float FloatParam::value() const {
	return static_cast<float>(_ui->spbParam->value());
}

void FloatParam::setValue(const float &val) {
	_ui->spbParam->setValue(static_cast<int>(val));
}
