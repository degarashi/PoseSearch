#include "directionparam3d.h"
#include <QTimer>
#include "aux_f/exception.hpp"
#include "aux_f/math.hpp"
#include "ui_directionparam_3d.h"

DirectionParam3D::DirectionParam3D(QWidget *parent) : DirectionParam3D({1, 0, 0}, parent) {
}
DirectionParam3D::DirectionParam3D(const QVector3D &initial, QWidget *parent) :
	ParamBaseT(parent), _ui(new Ui::DirectionParam3D) {
	if (initial.isNull())
		throw dg::InvalidInput("invalid initial vector");

	_ui->setupUi(this);
	_ui->yaw->setName("Yaw");
	_ui->yaw->initManipulator(false);
	_ui->pitch->setName("Pitch");
	_ui->pitch->initManipulator(true);

	// 初期値の設定（非同期実行）
	QTimer::singleShot(0, this, [this, initial] { setValue(initial); });
}

QVector3D DirectionParam3D::value() const {
	const float yaw = _ui->yaw->result().toFloat();
	const float pitch = _ui->pitch->result().toFloat();
	return dg::YawPitchToVec(yaw, pitch);
}

void DirectionParam3D::setValue(const QVector3D &vec) {
	qDebug() << vec;
	const auto yawPitch = dg::VecToYawPitch(vec);
	qDebug() << yawPitch.first << yawPitch.second;
	_ui->yaw->setValue(yawPitch.first);
	_ui->pitch->setValue(yawPitch.second);
}
