#include "my_settings.hpp"

MySettings::MySettings(const QString &path) : _settings(path, QSettings::IniFormat) {
}

namespace {
	const QString EntryStr[] = {
		"database/fileName",
	};
}

QVariant MySettings::getValue(const Entry entry) const {
	if (entry == Entry::DBFileName) {
		return _settings.value(GetEntryStr(entry));
	}
	return {};
}

const QString &MySettings::GetEntryStr(Entry entry) {
	return EntryStr[static_cast<int>(entry)];
}
void MySettings::setValue(Entry entry, const QVariant &val) {
	_settings.setValue(GetEntryStr(entry), val);
}

void MySettings::clearValue(Entry entry) {
	_settings.remove(GetEntryStr(entry));
}
