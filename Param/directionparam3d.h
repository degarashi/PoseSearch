#pragma once
#include <QVector3D>
#include "base.h"

namespace Ui {
	class DirectionParam3D;
}

class DirectionParam3D : public ParamBaseT<QVector3D> {
		Q_OBJECT
	public:
		explicit DirectionParam3D(QWidget *parent = nullptr);
		explicit DirectionParam3D(const QVector3D &initial, QWidget *parent = nullptr);

		QVector3D value() const override;
		void setValue(const QVector3D &vec) override;

	private:
		std::shared_ptr<Ui::DirectionParam3D> _ui;
};
