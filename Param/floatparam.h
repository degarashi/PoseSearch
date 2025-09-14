#pragma once
#include "base.h"

namespace Ui {
	class FloatParam;
}

class FloatParam : public ParamBaseT<float> {
		Q_OBJECT

	public:
		/**
		 * @brief コンストラクタ
		 * @param parent 親ウィジェット
		 */
		explicit FloatParam(float initial, QWidget *parent = nullptr);

		float value() const override;
		void setValue(const float &val) override;

	private:
		std::shared_ptr<Ui::FloatParam> _ui;
};
