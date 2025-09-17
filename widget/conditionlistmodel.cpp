#include "conditionlistmodel.hpp"
#include "condition/condition.hpp"

bool ConditionListModel::setData(const QModelIndex &index, const QVariant &value, const int role) {
	if (!index.isValid())
		return false;

	switch (role) {
		case Qt::CheckStateRole:
			_data[index.row()].enabled = value.toBool();
			// setData でデータが変更された際にdataChanged シグナルを発行する
			emit dataChanged(index, index, QList<int>{Qt::CheckStateRole});
			return true;
	}
	return false;
}

bool ConditionListModel::isConditionValid() const {
	for (const auto &item : _data) {
		if (item.enabled)
			return true;
	}
	return false;
}

ConditionListModel::ConditionListModel(QObject *parent) : QAbstractListModel(parent) {
}

int ConditionListModel::rowCount(const QModelIndex &parent) const {
	Q_UNUSED(parent)
	return _data.size(); // 条件リストのサイズを行数として返す
}

void ConditionListModel::addCondition(const Condition_SP &cond) {
	const int newRow = _data.size();
	// データが変更されたことをモデルに通知
	beginInsertRows({}, newRow, newRow);
	_data.emplace_back(cond, true);
	endInsertRows();
	// rowsInserted シグナルを発行する方が一般的で、モデルの内部状態をより正確に反映できる
	const auto idx = index(newRow, 0, {});
	emit dataChanged(idx, // 変更されたアイテムの開始インデックス
					 idx, // 変更されたアイテムの終了インデックス
					 {Qt::DisplayRole, Qt::UserRole} // 変更されたロールのリスト
	);
}

void ConditionListModel::clear() {
	beginResetModel();
	_data.clear();
	endResetModel();
}

const ConditionListModel::Data &ConditionListModel::data() const {
	return _data;
}

QVariant ConditionListModel::data(const QModelIndex &index, const int role) const {
	if (index.isValid()) {
		// 行がリストの範囲内かチェック
		if (index.row() < _data.size()) {
			// 対象の条件を取得
			const auto &ent = _data[index.row()];
			switch (role) {
				// 条件のテキストを返す
				case Qt::DisplayRole:
					return ent.cond->textPresent();
				// 条件オブジェクトへのポインタ
				case Qt::UserRole:
					// QVariant::fromValue() を使用してポインタを直接格納する
					return QVariant::fromValue(ent.cond.get());
				// チェックボックス(Conditionの有効化フラグ)
				case Qt::CheckStateRole:
					return ent.enabled ? Qt::CheckState::Checked : Qt::CheckState::Unchecked;
				default:;
			}
		}
	}
	// 無効なインデックスやロールの場合は空のバリアントを返す
	return {};
}

bool ConditionListModel::removeRows(const int row, const int count, const QModelIndex &parent) {
	// parent インデックスが有効な場合、その親に対応する行を削除する必要がある
	// ここでは常にルートノードを想定しているため、parent は無効であるべき
	if (!parent.isValid()) {
		if (row < 0 || row + count > _data.size()) {
			return false; // 無効な範囲の削除は許可しない
		}

		// 行削除を開始
		beginRemoveRows(parent, row, row + count - 1);
		// _dataからインデックスrowからcount個数分の要素を削除する
		{
			const auto itr = _data.begin() + row;
			_data.erase(itr, itr + count);
		}
		// 行削除を終了
		endRemoveRows();
		// 削除が成功したことを示す
		return true;
	}
	return false; // 無効な親インデックスの場合は false を返す
}

Qt::ItemFlags ConditionListModel::flags(const QModelIndex &index) const {
	// 無効なインデックスに対して呼び出した際のクラッシュを防ぐ
	if (!index.isValid())
		return Qt::NoItemFlags;
	return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable;
}
