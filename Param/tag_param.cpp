#include "tag_param.h"
#include "aux_f/exception.hpp"
#include "my_db.hpp"
#include "ui_tag_param.h"

TagParam::TagParam(const QString &initial, QWidget *parent) : ParamBaseT(parent), _ui(new Ui::TagParam) {
	_ui->setupUi(this);
	// タグ文字列を取得
	_ui->cbTag->addItems(myDb_c.getTagList());

	if (!initial.isEmpty())
		setValue(initial);
	else {
		// cbTagはデフォルトで0番を選択
		_ui->cbTag->setCurrentIndex(0);
	}
}

QString TagParam::value() const {
	return _ui->cbTag->currentText();
}

void TagParam::setValue(const QString &val) {
	const int index = _ui->cbTag->findText(val);
	if (index != -1) {
		_ui->cbTag->setCurrentIndex(index);
	}
	else {
		// 見つからない場合はエラー
		throw dg::InvalidInput(QString("invalid tag %1").arg(val));
	}
}
