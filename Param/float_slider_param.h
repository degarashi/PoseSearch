#pragma once
#include "aux_f/value.hpp"
#include "base.h"

namespace Ui {
	class float_slider_param;
}

class FloatSliderParam : public ParamBaseT<float> {
		Q_OBJECT
	public:
		explicit FloatSliderParam(const dg::Range<float>& range, float initial, QWidget *parent = nullptr);
		float value() const override;
		void setValue(const float &val) override;

	private:
		std::shared_ptr<Ui::float_slider_param> _ui;
		// (UIではない)実際の値の範囲
		dg::Range<float> _range;
};
