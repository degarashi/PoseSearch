#include "conditionlist_dlg.hpp"
#include <QApplication>
#include <QPainter>
#include <QSlider>
#include <QStyleOptionProgressBar>
#include <QStyledItemDelegate>
#include "aux_f_q/q_value.hpp"
#include "cond_data.hpp"
#include "conditionmodel.hpp"

// 表示時はプログレスバー
void SliderDelegate::paint(QPainter *const painter, const QStyleOptionViewItem &option,
						   const QModelIndex &index) const {
	if (index.column() == static_cast<int>(ConditionModel::Column::Ratio)) {
		const auto p = dg::ConvertQV<CondParam>(index.model()->data(index, Qt::EditRole));
		const int percent = static_cast<int>(dg::Remap<float>(p.current, p.range.min, p.range.max, 0.0f, 100.0f));

		QStyleOptionProgressBar bar;
		bar.rect = option.rect;
		bar.minimum = 0;
		bar.maximum = 100;
		bar.progress = percent;
		bar.text = QString::number(p.current, 'f', 2);
		bar.textVisible = true;

		QApplication::style()->drawControl(QStyle::CE_ProgressBar, &bar, painter);
	}
	else {
		QStyledItemDelegate::paint(painter, option, index);
	}
}

// 編集開始時はスライダーを生成
QWidget *SliderDelegate::createEditor(QWidget *const parent, const QStyleOptionViewItem &,
									  const QModelIndex &index) const {
	if (index.column() != static_cast<int>(ConditionModel::Column::Ratio))
		return nullptr;

	auto *const slider = new QSlider(Qt::Horizontal, parent);
	slider->setMinimum(0);
	slider->setMaximum(1000); // 精度を上げたい場合は大きめに
	return slider;
}

// モデル値をスライダーに反映
void SliderDelegate::setEditorData(QWidget *const editor, const QModelIndex &index) const {
	if (index.column() != static_cast<int>(ConditionModel::Column::Ratio))
		return;

	const auto p = dg::ConvertQV<CondParam>(index.model()->data(index, Qt::EditRole));
	auto *const slider = qobject_cast<QSlider *>(editor);
	const float mapped = dg::Remap<float>(p.current, p.range.min, p.range.max, 0.0f, 1000.0f);
	slider->setValue(std::lround(mapped));
}

// スライダー値をモデルに書き戻す
void SliderDelegate::setModelData(QWidget *const editor, QAbstractItemModel *const model,
								  const QModelIndex &index) const {
	if (index.column() != static_cast<int>(ConditionModel::Column::Ratio))
		return;

	const auto p = dg::ConvertQV<CondParam>(index.model()->data(index, Qt::EditRole));
	auto *const slider = qobject_cast<QSlider *>(editor);
	const float value = dg::Remap<float>(slider->value(), 0.0f, 1000.0f, p.range.min, p.range.max);
	model->setData(index, value, Qt::EditRole);
}

// セル領域に合わせてエディタの位置と大きさを更新
void SliderDelegate::updateEditorGeometry(QWidget *const editor, const QStyleOptionViewItem &option,
										  const QModelIndex &) const {
	editor->setGeometry(option.rect);
}
