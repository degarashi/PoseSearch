#pragma once

template <typename T>
class StaticClassBase {
	public:
		static auto &StaticClass() {
			static T s_instance;
			return s_instance;
		}
};
