#pragma once
#include "base.h"
#include "ui_paramwrapper.h"

// 本体となるParamBaseにヘッダ(見出し)を付けたもの
template <typename T>
class ParamWrapper : public ParamBaseT<T> {
	public:
		/**
		 * @brief ParamWrapperクラスのコンストラクタ
		 *
		 * UIをセットアップし、指定されたパラメータと名前を設定
		 *
		 * @param p 設定するParamBaseオブジェクトへのポインタ
		 * @param name 表示するパラメータ名
		 * @param parent 親ウィジェットへのポインタ
		 */
		explicit ParamWrapper(ParamBaseT<T> *p, const QString &name, QWidget *parent = nullptr) :
			ParamBaseT<T>(parent), _ui(new Ui::ParamWrapper) {
			_ui->setupUi(this);
			_setParam(p);
			_ui->label->setText(name);
		}
		T value() const override {
			auto *ret = static_cast<const ParamBaseT<T>*>(_ui->placeHolder);
			return ret->value();
		}
		void setValue(const T& val) override {
			auto *ret = static_cast<ParamBaseT<T>*>(_ui->placeHolder);
			ret->setValue(val);
		}

	private:
		std::shared_ptr<Ui::ParamWrapper> _ui;
		/**
		 * @brief パラメータを設定
		 *
		 * 現在のパラメータを削除し、新しいパラメータを設定
		 *
		 * @param p 設定するParamBaseオブジェクトへのポインタ
		 */
		void _setParam(ParamBase *p) {
			// layoutの最後のウィジェットが _ui->placeHolder と一致する前提
			auto *ph = _ui->horizontalLayout->takeAt(_ui->horizontalLayout->count() - 1);
			Q_ASSERT(_ui->placeHolder == ph->widget());
			delete ph;
			_ui->horizontalLayout->addWidget(p);
			_ui->placeHolder = p;
		}
};
