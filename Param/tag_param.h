#pragma once

#include "param/base.h"

namespace Ui {
	class TagParam;
}

class TagParam : public ParamBaseT<QString> {
		Q_OBJECT

	public:
		explicit TagParam(const QString &initial = {}, QWidget *parent = nullptr);
		QString value() const override;
		void setValue(const QString& val) override;

	private:
		std::shared_ptr<Ui::TagParam> _ui;
};
