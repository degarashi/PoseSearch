#pragma once
#include <QAbstractItemModel>
#include <cereal/types/memory.hpp>
#include <cereal/types/vector.hpp>

class Condition;
using Condition_SP = std::shared_ptr<Condition>;
class ConditionListModel : public QAbstractListModel {
		Q_OBJECT
	private:
		struct DataEnt {
				Condition_SP cond;
				bool enabled;

				template <typename Ar>
				void serialize(Ar &ar) {
					ar(cond, enabled);
				}
		};
		using Data = std::vector<DataEnt>;
		Data _data;

	public:
		explicit ConditionListModel(QObject *parent = nullptr);

		// 指定された親インデックスに対応する子アイテムの行数を返す
		virtual int rowCount(const QModelIndex &parent) const override;
		// 指定されたモデルインデックスとロールに対応するデータを返す
		virtual QVariant data(const QModelIndex &index, int role) const override;
		// 指定された行範囲のアイテムを削除
		bool removeRows(int row, int count, const QModelIndex &parent) override;
		bool setData(const QModelIndex &index, const QVariant &value, int role) override;
		Qt::ItemFlags flags(const QModelIndex &index) const override;

		void addCondition(const Condition_SP &cond);
		void clear();

		template <typename Ar>
		void serialize(Ar &ar) {
			ar(_data);
			if constexpr (Ar::is_loading::value) {
				// モデルが更新されたことを知らせる
				emit dataChanged(index(0), index(_data.size()), {Qt::DisplayRole, Qt::UserRole});
			}
		}

		// 条件リスト全体を取得
		const Data &data() const;
		bool isConditionValid() const;
};
