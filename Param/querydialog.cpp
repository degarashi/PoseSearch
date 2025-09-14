#include "querydialog.h"
#include <QFrame>
#include "param/base.h"
#include "ui_querydialog.h"

QueryDialog::QueryDialog(QWidget *parent) : QDialog(parent), _ui(std::make_shared<Ui::QueryDialog>()) {
	_ui->setupUi(this);
	layout()->setSizeConstraint(QLayout::SetFixedSize);
}

void QueryDialog::addParam(QWidget *param) {
	auto *v = _ui->verticalLayout;
	// paramウィジェットを追加
	v->insertWidget(v->count() - 1, param);
	// 区切り線
	auto *line = new QFrame(this);
	line->setFrameShape(QFrame::HLine);
	line->setFrameShadow(QFrame::Sunken);
	v->insertWidget(v->count() - 1, line);
}

QVariantList QueryDialog::result() const {
	auto *v = _ui->verticalLayout;
	int nEnt = v->count() - 1;
	Q_ASSERT((nEnt & 1) == 0);
	nEnt /= 2;

	QVariantList vl;
	for (int i = 0; i < nEnt; i++) {
		auto *param = qobject_cast<const ParamBase *>(v->itemAt(i * 2)->widget());
		vl.append(param->result());
	}
	return vl;
}
