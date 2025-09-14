#pragma once
#include "base.h"

namespace Ui { class DirectionParam2D; }

class DirectionParam2D : public ParamBaseT<int> {
	Q_OBJECT
	public:
		explicit DirectionParam2D(QWidget *parent = nullptr);
		explicit DirectionParam2D(int initial, QWidget *parent = nullptr);
		void initManipulator(bool bPitch);

		int value() const override;
		void setValue(const int& val) override;

		void setName(const QString& name);

	private:
		std::shared_ptr<Ui::DirectionParam2D> _ui;
};
