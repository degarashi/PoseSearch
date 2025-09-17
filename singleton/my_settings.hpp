#pragma once
#include <QString>
#include <QSettings>
#include "singleton.hpp"

#define mySet (MySettings::Get())
#define mySet_c (MySettings::GetC())

class MySettings : public dg::Singleton<MySettings> {
	private:
		QSettings	_settings;
	public:
		enum class Entry {
			DBFileName,
		};
		static const QString& GetEntryStr(Entry entry);
		MySettings(const QString &path);

		QVariant getValue(Entry entry) const;
		void setValue(Entry entry, const QVariant& val);
		void clearValue(Entry entry);
};
