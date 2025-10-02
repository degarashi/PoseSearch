#include "query.hpp"
#include "error.hpp"
#include <QSqlQuery>

namespace dg::sql {
	// prepareで分離していたパラメータを全部文字列として展開(デバッグ用)
	QString MakeQueryString(const QSqlQuery &q) {
		auto ret = q.executedQuery();
		auto vals = q.boundValues();
		for(int i=0 ; i<vals.size() ; i++) {
			const auto val = vals[i];
			const auto key = q.boundValueName(i);
			if(val.typeId() == QMetaType::QString)
				ret.replace(key, "'" + val.toString() + "'");
			else
				ret.replace(key, val.toString());
		}
		return ret;
	}
	void Query(QSqlQuery& q) {
		#ifdef QUERY_AS_STRING
			if(!q.exec(MakeQueryString(q)))
		#else
			if(!q.exec())
		#endif
			ThrowError(q);
	}
	void Batch(QSqlQuery& q) {
		if(!q.execBatch())
			ThrowError(q);
	}
}
