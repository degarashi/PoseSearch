#pragma once
#include <QWidget>
#include "param/base.h"

namespace Ui {
	class TextParam;
}

class TextParam : public ParamBaseT<QString> {
		Q_OBJECT

	public:
		explicit TextParam(const QString &initialValue = {}, QWidget *parent = nullptr);
		QString value() const override;
		void setValue(const QString &s) override;

	private:
		std::shared_ptr<Ui::TextParam> _ui;
};
