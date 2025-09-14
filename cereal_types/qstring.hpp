#pragma once
#include <QString>

namespace cereal {
	template <class Archive>
	void load(Archive &ar, QString &s) {
		std::string temp;
		ar(temp);
		s = QString::fromStdString(temp);
	}
	template <class Archive>
	void save(Archive &ar, const QString &s) {
		ar(s.toStdString());
	}
} // namespace cereal
