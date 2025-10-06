#pragma once

#include <QAbstractItemModel>
#include <QHeaderView>
#include <QModelIndex>
#include <QVariant>
#include <QVector>
#include <cereal/types/memory.hpp>
#include <cereal/types/vector.hpp>
#include "condition/condition.hpp"

class Condition;
using Condition_SP = std::shared_ptr<Condition>;
class ConditionModel : public QAbstractTableModel {
		Q_OBJECT

	public:
		struct Entry {
				Condition_SP cond;
				bool enabled;

				// cereal用にデフォルト構築を許可
				Entry();
				explicit Entry(Condition_SP c, bool en = true);

				template <typename Ar>
				void serialize(Ar &ar) {
					ar(cond, enabled);
					Q_ASSERT(cond);
				}
		};
		using Data = std::vector<Entry>;

		enum class Column : int {
			Enabled,
			Slider,
			Title,
			Info,
			_Count,
		};

	public:
		explicit ConditionModel(QObject *parent = nullptr);

		// QAbstractItemModel overrides
		int rowCount(const QModelIndex &parent = QModelIndex()) const override;
		int columnCount(const QModelIndex &parent = QModelIndex()) const override;
		QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
		bool setData(const QModelIndex &index, const QVariant &value, int role) override;
		Qt::ItemFlags flags(const QModelIndex &index) const override;
		QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
		bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
		QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;

		// 便利関数
		void addCondition(const Condition_SP &cond);
		void clear();
		std::vector<QHeaderView::ResizeMode> getResizeMode() const noexcept;

		template <typename Ar>
		void serialize(Ar &ar) {
			if constexpr (Ar::is_loading::value) {
				// ロード時はモデル全体が入れ替わるため、リセット通知で安全に更新する
				beginResetModel();
				ar(_data);
#ifdef QT_DEBUG
				for (const auto &ent : _data)
					Q_ASSERT(ent.cond);
#endif
				endResetModel();
			}
			else {
				ar(_data);
			}
		}

		// 条件リスト全体を取得
		const Data &data() const;
		bool isConditionValid() const;

	private:
		Data _data;
};
