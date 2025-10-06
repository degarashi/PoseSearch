#include "conditionlist_dlg.hpp"
#include "aux_f/value.hpp"
#include "aux_f_q/q_value.hpp"
#include "cond_data.hpp"

namespace {
	constexpr int MIN_V = 0;
	constexpr int MAX_V = 1000;
} // namespace

// 編集開始時にセル用エディタを生成
// parent 引数は生成エディタの親ウィジェット参照
// QStyleOptionViewItem 引数は表示オプション参照
QWidget *SliderDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &) const {
	auto *const slider = new QSlider(Qt::Horizontal, parent);
	slider->setMinimum(MIN_V);
	slider->setMaximum(MAX_V);

	// 背景をセルと同じ色で塗る
	slider->setAutoFillBackground(true);
	QPalette pal = slider->palette();
	pal.setColor(QPalette::Window, pal.color(QPalette::Base));
	slider->setPalette(pal);

	return slider;
}

// モデルからエディタへ値を反映
void SliderDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const {
	// スライダー位置や範囲などの初期値設定
	const auto p = dg::ConvertQV<CondParam>(index.model()->data(index, Qt::EditRole));
	QSlider *const slider = qobject_cast<QSlider *>(editor);
	// intへマッピングする
	const float value = dg::Remap<float>(p.current, p.range.min, p.range.max, MIN_V, MAX_V);
	slider->setValue(std::round(value));
}

// エディタからモデルへ値を書き戻す
void SliderDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const {
	const auto p = dg::ConvertQV<CondParam>(index.model()->data(index, Qt::EditRole));
	QSlider *const slider = qobject_cast<QSlider *>(editor);
	// int -> floatへマッピング
	const auto value = dg::Remap<float>(slider->value(), MIN_V, MAX_V, p.range.min, p.range.max);
	model->setData(index, value, Qt::EditRole);
}

// セル領域に合わせてエディタの位置と大きさを更新
void SliderDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
										  const QModelIndex &) const {
	editor->setGeometry(option.rect);
}
