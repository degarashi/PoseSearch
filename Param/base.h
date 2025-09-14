#pragma once
#include <QWidget>

//! 姿勢検索のパラメータベース
class ParamBase : public QWidget {
		Q_OBJECT

	public:
		explicit ParamBase(QWidget *parent = nullptr);
		virtual QVariant result() const = 0;
};

template <typename T>
class ParamBaseT : public ParamBase {
	public:
		using ParamBase::ParamBase;

		virtual T value() const = 0;
		virtual void setValue(const T &val) = 0;

		QVariant result() const override final {
			return QVariant::fromValue(value());
		}
};
