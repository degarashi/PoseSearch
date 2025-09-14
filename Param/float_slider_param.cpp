#include "float_slider_param.h"
#include <QTimer>
#include "aux_f/value.hpp"
#include "ui_float_slider_param.h"

FloatSliderParam::FloatSliderParam(const dg::Range<float> &range, const float initial, QWidget *parent) :
	ParamBaseT(parent), _range(range), _ui(new Ui::float_slider_param) {
	_ui->setupUi(this);

	// スライダーの値が変更されたときに、ラベルのテキストを更新するシグナルとスロットを接続
	connect(_ui->slider, &QAbstractSlider::valueChanged, this,
			[this]() { _ui->label->setText(QString::number(value(), 'f', 2)); }); // 現在の値を小数点以下2桁で表示

	// UIのスライダー値はあくまでUI操作用なので適当にセット
	// (これだと1000刻みになる)
	_ui->slider->setRange(0, 1000);
	_ui->slider->setValue(500);

	// 初期値の設定（非同期実行）
	QTimer::singleShot(0, this, [this, initial]() { setValue(initial); });
}

float FloatSliderParam::value() const {
	auto &sl = _ui->slider;
	// 正規化された値を、設定された最小値と最大値の範囲にマッピングして返す
	return dg::Remap<float>(sl->value(), sl->minimum(), sl->maximum(), _range.min, _range.max);
}

void FloatSliderParam::setValue(const float &val) {
	auto &sl = _ui->slider;
	// 設定された最小値と最大値の範囲の値を、UIのスライダーの範囲にマッピングしてセット
	sl->setValue(dg::Remap<float>(val, _range.min, _range.max, sl->minimum(), sl->maximum()));
}
