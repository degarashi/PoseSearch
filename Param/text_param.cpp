#include "text_param.h"
#include "ui_text_param.h"

TextParam::TextParam(const QString &initialValue, QWidget *parent) : ParamBaseT(parent), _ui(new Ui::TextParam) {
	_ui->setupUi(this);
	// 初期値を設定
	setValue(initialValue);
}

QString TextParam::value() const {
	return _ui->textParam->text();
}

void TextParam::setValue(const QString &s) {
	_ui->textParam->setText(s);
}
