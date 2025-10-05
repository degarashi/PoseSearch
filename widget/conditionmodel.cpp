#include "conditionmodel.hpp"
#include "condition/condition.hpp"

ConditionModel::Entry::Entry(Condition_SP c, bool en) : cond(std::move(c)), enabled(en) {
	Q_ASSERT(cond);
}
ConditionModel::Entry::Entry() : cond(nullptr), enabled(false) {
}

ConditionModel::ConditionModel(QObject *parent) : QAbstractTableModel(parent) {
}

int ConditionModel::rowCount(const QModelIndex &parent) const {
	Q_UNUSED(parent)
	// 条件リストのサイズを行数として返す
	return _data.size();
}

int ConditionModel::columnCount(const QModelIndex &parent) const {
	Q_UNUSED(parent)
	// テーブル列数
	return static_cast<int>(Column::_Count);
}

QVariant ConditionModel::data(const QModelIndex &index, const int role) const {
	// インデックスの有効性と範囲チェック
	if (!index.isValid())
		return {};

	const int row = index.row();
	const auto colIdx = static_cast<Column>(index.column());

	if (row < 0 || row >= _data.size() || index.column() < 0 || index.column() >= static_cast<int>(Column::_Count))
		return {};

	// 対象の条件エントリを取得
	const auto &ent = _data[row];
	Q_ASSERT(ent.cond);

	switch (role) {
		// 表示テキスト
		case Qt::DisplayRole:
			if (colIdx == Column::Text)
				return ent.cond->textPresent();
			// チェック列の表示文字列は不要なので空を返す
			return {};

		// チェックボックス状態
		case Qt::CheckStateRole:
			if (colIdx == Column::Enabled)
				return ent.enabled ? Qt::CheckState::Checked : Qt::CheckState::Unchecked;
			return {};

		// 条件オブジェクトへのポインタ (列に依らず行のメタ情報として返す)
		case Qt::UserRole:
			// QVariant::fromValue() を使用してポインタを直接格納する
			return QVariant::fromValue(ent.cond.get());

		default:
			return {};
	}
}

bool ConditionModel::setData(const QModelIndex &index, const QVariant &value, const int role) {
	if (!index.isValid())
		return false;

	const int row = index.row();
	const auto col = static_cast<Column>(index.column());
	if (row < 0 || row >= _data.size() || index.column() < 0 || index.column() >= static_cast<int>(Column::_Count))
		return false;

	switch (role) {
		// チェックボックス変更
		case Qt::CheckStateRole: {
			if (col != Column::Enabled)
				return false;

			const bool newEnabled = value.toBool();
			if (_data[row].enabled == newEnabled)
				return true; // 変更なし

			_data[row].enabled = newEnabled;
			// setData でデータが変更された際に dataChanged シグナルを発行する
			emit dataChanged(index, index, QList<int>{Qt::CheckStateRole});
			return true;
		}
		default:
			break;
	}
	return false;
}

Qt::ItemFlags ConditionModel::flags(const QModelIndex &index) const {
	// 無効なインデックスに対して呼び出した際のクラッシュを防ぐ
	if (!index.isValid())
		return Qt::NoItemFlags;

	const auto col = static_cast<Column>(index.column());

	// 列ごとのフラグ設定
	switch (col) {
		case Column::Text:
			// テキスト列は選択可能・有効
			return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
		case Column::Enabled:
			// チェック列は選択可能・有効・ユーザーチェック可能
			return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable;
		default:
			return Qt::NoItemFlags;
	}
}

QVariant ConditionModel::headerData(int section, Qt::Orientation orientation, int role) const {
	// ヘッダー表示 (水平のみ)
	if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
		return {};

	const auto col = static_cast<Column>(section);
	switch (col) {
		case Column::Text:
			return QStringLiteral("Condition"); // 条件のテキスト
		case Column::Enabled:
			return QStringLiteral("Enabled"); // 有効フラグ
		default:
			return {};
	}
}

bool ConditionModel::removeRows(const int row, const int count, const QModelIndex &parent) {
	// 常にルートノードを想定しているため、parent は無効であるべき
	if (parent.isValid())
		return false;

	// 範囲チェック
	if (row < 0 || count <= 0 || row + count > _data.size())
		return false;

	// 行削除を開始
	beginRemoveRows(parent, row, row + count - 1);

	// _data からインデックス row から count 個数分の要素を削除する
	const auto first = _data.begin() + row;
	_data.erase(first, first + count);

	// 行削除を終了
	endRemoveRows();

	// 削除が成功したことを示す
	return true;
}

void ConditionModel::addCondition(const Condition_SP &cond) {
	Q_ASSERT(cond);
	const int newRow = _data.size();

	// 行追加をモデルに通知
	beginInsertRows(QModelIndex(), newRow, newRow);

	// 追加時は既定で有効にする
	_data.push_back(Entry{cond, true});

	// 追加終了
	endInsertRows();

	// 挿入後に当該行の表示/チェック状態を明示的に更新通知 (必要最小限のロール)
	const auto idxText = index(newRow, static_cast<int>(Column::Text));
	const auto idxEnabled = index(newRow, static_cast<int>(Column::Enabled));
	emit dataChanged(idxText, idxText, {Qt::DisplayRole, Qt::UserRole});
	emit dataChanged(idxEnabled, idxEnabled, {Qt::CheckStateRole});
}

void ConditionModel::clear() {
	beginResetModel();
	_data.clear();
	endResetModel();
}

const ConditionModel::Data &ConditionModel::data() const {
	return _data;
}

bool ConditionModel::isConditionValid() const {
	// 少なくとも一つの条件が有効であれば true
	for (const auto &item : _data) {
		if (item.enabled)
			return true;
	}
	return false;
}

QModelIndex ConditionModel::index(int row, int column, const QModelIndex &parent) const {
	if (parent.isValid())
		return {};
	if (row < 0 || row >= static_cast<int>(_data.size()))
		return {};
	if (column < 0 || column >= static_cast<int>(Column::_Count))
		return {};
	return createIndex(row, column);
}
