#include "conditionmodel.hpp"
#include "aux_f_q/q_value.hpp"
#include "cond_data.hpp"
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
	return static_cast<int>(_data.size());
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

	if (row < 0 || row >= static_cast<int>(_data.size()) || index.column() < 0 ||
		index.column() >= static_cast<int>(Column::_Count))
		return {};

	// 対象の条件エントリを取得
	const auto &ent = _data[static_cast<std::size_t>(row)];
	Q_ASSERT(ent.cond);

	switch (role) {
		case Qt::EditRole:
			if (colIdx == Column::Ratio)
				return QVariant::fromValue(CondParam{ent.cond->getRatio(), ent.cond->getRatioRange()});
			return {};

		case Qt::DisplayRole:
			switch (colIdx) {
				case Column::Title:
					return ent.cond->dialogName();
				case Column::Info:
					return ent.cond->textPresent();
				default:
					return {};
			}

		// プログレスバー用の値を返す
		case Qt::UserRole:
			if (colIdx == Column::Ratio) {
				// 0〜100 の整数値に変換して返す
				const float ratio = ent.cond->getRatio();
				const int percent = static_cast<int>(ratio * 100.0f);
				return percent;
			}
			// 条件オブジェクトへのポインタを返す
			return QVariant::fromValue(ent.cond.get());

		case Qt::CheckStateRole:
			if (colIdx == Column::Enabled)
				return ent.enabled ? Qt::CheckState::Checked : Qt::CheckState::Unchecked;
			return {};

		default:
			return {};
	}
}

bool ConditionModel::setData(const QModelIndex &index, const QVariant &value, const int role) {
	if (!index.isValid())
		return false;

	const int row = index.row();
	const auto col = static_cast<Column>(index.column());
	if (row < 0 || row >= static_cast<int>(_data.size()) || index.column() < 0 ||
		index.column() >= static_cast<int>(Column::_Count))
		return false;

	auto &ent = _data[static_cast<std::size_t>(row)];
	switch (role) {
		case Qt::EditRole:
			ent.cond->setRatio(dg::ConvertQV<float>(value));
			emit dataChanged(index, index, {Qt::UserRole});
			return true;
		case Qt::CheckStateRole: {
			if (col != Column::Enabled)
				return false;

			const bool newEnabled = value.toBool();
			if (ent.enabled == newEnabled)
				return true; // 変更なし

			ent.enabled = newEnabled;
			emit dataChanged(index, index, {Qt::CheckStateRole});
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

	switch (col) {
		case Column::Title:
		case Column::Info:
			return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
		case Column::Enabled:
			return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable;
		case Column::Ratio:
			// プログレスバー表示用に編集可能にしておく
			return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
		default:
			return Qt::NoItemFlags;
	}
}

QVariant ConditionModel::headerData(int section, Qt::Orientation orientation, int role) const {
	if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
		return {};

	const auto col = static_cast<Column>(section);
	switch (col) {
		case Column::Title:
			return QStringLiteral("Name");
		case Column::Ratio:
			return QStringLiteral("Progress");
		case Column::Enabled:
			return QStringLiteral("Enabled");
		case Column::Info:
			return QStringLiteral("Info");
		default:
			return {};
	}
}

bool ConditionModel::removeRows(const int row, const int count, const QModelIndex &parent) {
	// 常にルートノードを想定しているため、parent は無効であるべき
	if (parent.isValid())
		return false;

	// 範囲チェック
	if (row < 0 || count <= 0 || row + count > static_cast<int>(_data.size()))
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
	const int newRow = static_cast<int>(_data.size());

	// 行追加をモデルに通知
	beginInsertRows(QModelIndex(), newRow, newRow);

	// 追加時は既定で有効にする
	_data.push_back(Entry{cond, true});

	// 追加終了
	endInsertRows();

	// 挿入後に当該行の表示/チェック状態を明示的に更新通知 (必要最小限のロール)
	const auto idxEnabled = index(newRow, static_cast<int>(Column::Enabled));
	const auto idxTitle = index(newRow, static_cast<int>(Column::Title));
	const auto idxInfo = index(newRow, static_cast<int>(Column::Info));
	const auto idxRatio = index(newRow, static_cast<int>(Column::Ratio));
	emit dataChanged(idxEnabled, idxEnabled, {Qt::CheckStateRole});
	emit dataChanged(idxTitle, idxTitle, {Qt::DisplayRole, Qt::UserRole});
	emit dataChanged(idxInfo, idxInfo, {Qt::DisplayRole, Qt::UserRole});
	emit dataChanged(idxRatio, idxRatio, {Qt::UserRole});
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
std::vector<QHeaderView::ResizeMode> ConditionModel::getResizeMode() const noexcept {
	return {
		QHeaderView::ResizeToContents,
		QHeaderView::Interactive,
		QHeaderView::ResizeToContents,
		QHeaderView::Stretch,
	};
}
