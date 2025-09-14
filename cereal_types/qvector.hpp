#pragma once
#include <QVector2D>
#include <QVector3D>
#include <QVector4D>

namespace cereal {
	template <class Archive>
	void save(Archive &ar, const QVector2D &vec) {
		ar(vec.x(), vec.y());
	}

	template <class Archive>
	void load(Archive &ar, QVector2D &vec) {
		float x, y;
		ar(x, y);
		vec.setX(x);
		vec.setY(y);
	}

	template <class Archive>
	void save(Archive &ar, const QVector3D &vec) {
		ar(vec.x(), vec.y(), vec.z());
	}

	template <class Archive>
	void load(Archive &ar, QVector3D &vec) {
		float x, y, z;
		ar(x, y, z);
		vec.setX(x);
		vec.setY(y);
		vec.setZ(z);
	}

	template <class Archive>
	void save(Archive &ar, const QVector4D &vec) {
		ar(vec.x(), vec.y(), vec.z(), vec.w());
	}

	template <class Archive>
	void load(Archive &ar, QVector4D &vec) {
		float x, y, z, w;
		ar(x, y, z, w);
		vec.setX(x);
		vec.setY(y);
		vec.setZ(z);
		vec.setW(w);
	}

} // namespace cereal
