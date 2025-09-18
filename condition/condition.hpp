#pragma once
#include <QJsonArray>
#include <QJsonObject>
#include <QVector3D>
#include <cereal/types/polymorphic.hpp>
#include <cereal_types/qstring.hpp>
#include <cereal_types/qvector.hpp>
#include "static_base.hpp"

QT_BEGIN_NAMESPACE
class QSqlDatabase;
QT_END_NAMESPACE

namespace dg::sql {
	class Database;
}
class QSqlQuery;
struct QuerySeed {
		using QueryPair = QPair<QString, QVariant>;
		using QueryParams = QVector<QueryPair>;
		QString queryText;
		QueryParams queryParams;
		float ratio;

		void setupParams(QSqlQuery &q, int limit) const;
		QSqlQuery exec(dg::sql::Database &db, const QString &qtext, int limit) const;
};

struct QueryParam {
		QString outputTableName;
		float ratio;
};
class Condition;
using Condition_SP = std::shared_ptr<Condition>;

class QueryDialog;
class Condition {
	public:
		// prepareした時のダイアログタイトル名
		virtual QString dialogName() const = 0;
		// テンプレConditionをModelへコピーする際に使用
		virtual Condition_SP clone() const = 0;
		// 条件の文字列表現(ConditionModel用)
		virtual QString textPresent() const = 0;
		// ダイアログのパラメータUIセットアップ(QueryDialogに追記)
		virtual void setupDialog(QueryDialog &dlg) const;
		// QueryDialogから返された変数リストの解釈
		virtual void loadParamFromDialog(const QVariantList &vl);

		virtual QuerySeed getSqlQuery(const QueryParam &param) const = 0;
		float getRatio() const noexcept;

		template <typename Ar>
		void serialize(Ar &ar) {
			ar(_ratio);
		}

	protected:
		void _clone(Condition &dst) const;
		QString _textPresent(const QString &src) const;

	private:
		float _ratio = 1.f;
};

#define DEF_FUNCS                                                                                                      \
	QString dialogName() const override;                                                                               \
	Condition_SP clone() const override;                                                                               \
	QString textPresent() const override;                                                                              \
	void setupDialog(QueryDialog &dlg) const override;                                                                 \
	void loadParamFromDialog(const QVariantList &vl) override;                                                         \
	QuerySeed getSqlQuery(const QueryParam &param) const override;

// 条件：胴体の方向
class Cond_BodyDir : public Condition, public StaticClassBase<Cond_BodyDir> {
	private:
		QVector3D _dir;

	public:
		Cond_BodyDir();
		DEF_FUNCS

		template <typename Ar>
		void serialize(Ar &ar) {
			ar(_dir);
			ar(cereal::base_class<Condition>(this));
		}
};
// 条件：ディレクトリによるタグ
class Cond_Tag : public Condition, public StaticClassBase<Cond_Tag> {
	private:
		QString _tagName;

	public:
		DEF_FUNCS

		template <typename Ar>
		void serialize(Ar &ar) {
			ar(_tagName);
			ar(cereal::base_class<Condition>(this));
		}
};
// 条件：胴体の方向(Yaw)
class Cond_BodyDirYaw : public Condition, public StaticClassBase<Cond_BodyDirYaw> {
	private:
		QVector2D _yawDir;

	public:
		Cond_BodyDirYaw();
		DEF_FUNCS

		template <typename Ar>
		void serialize(Ar &ar) {
			ar(_yawDir);
			ar(cereal::base_class<Condition>(this));
		}
};
// 条件：胴体の方向(Pitch)
class Cond_BodyDirPitch : public Condition, public StaticClassBase<Cond_BodyDirPitch> {
	private:
		int _pitch;

	public:
		Cond_BodyDirPitch();
		DEF_FUNCS

		template <typename Ar>
		void serialize(Ar &ar) {
			ar(_pitch);
			ar(cereal::base_class<Condition>(this));
		}
};
#undef DEF_FUNCS

QJsonArray VecToJArray(const QVector3D &v);
QString AttachGUID(QJsonObject &js);
