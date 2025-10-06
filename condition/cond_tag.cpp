#include "condition.hpp"
#include "param/paramwrapper.h"
#include "param/querydialog.h"
#include "param/tag_param.h"

QString Cond_Tag::dialogName() const {
	return "Directory Tag";
}

Condition_SP Cond_Tag::clone() const {
	auto ret = std::make_shared<Cond_Tag>();
	Condition::_clone(*ret);
	ret->_tagName = _tagName;
	return ret;
}

QString Cond_Tag::textPresent() const {
	return QString("tag: %1").arg(_tagName);
}

void Cond_Tag::setupDialog(QueryDialog &dlg) const {
	dlg.addParam(new ParamWrapper(new TagParam(_tagName), "Tag"));
	Condition::setupDialog(dlg);
}

void Cond_Tag::loadParamFromDialog(const QVariantList &vl) {
	_tagName = vl[0].toString();
	Condition::loadParamFromDialog(vl);
}

QuerySeed Cond_Tag::getSqlQuery(const QueryParam &param) const {
	return {QString("WITH %1 AS ("
					"SELECT poseId, 1.0 AS score "
					"FROM TagInfo "
					"INNER JOIN Tags "
					"	ON TagInfo.id = Tags.tagId "
					"WHERE TagInfo.name = :tag_name "
					"LIMIT :limit "
					")")
				.arg(param.outputTableName),
			{
				{":tag_name", _tagName},
			},
			param.ratio};
}
