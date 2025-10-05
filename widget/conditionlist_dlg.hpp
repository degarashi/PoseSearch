#include <QStyledItemDelegate>

// テーブルやリストのセル編集にスライダー型エディタを提供するデリゲートクラス
class SliderDelegate : public QStyledItemDelegate {
	Q_OBJECT // Qtのメタオブジェクト機構を有効化するマクロ
		public : using QStyledItemDelegate::QStyledItemDelegate; // 基底デリゲートのコンストラクタ群を継承する宣言

		QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &) const override;

		void setEditorData(QWidget *editor, const QModelIndex &index) const override;
		void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
		void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
								  const QModelIndex &) const override;
};
