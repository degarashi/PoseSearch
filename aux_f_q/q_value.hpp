#pragma once
#include <QVariant>
#include <type_traits>

namespace dg {
	namespace detail {
		bool _ConvertQV(const QVariant &v, bool *);
		float _ConvertQV(const QVariant &v, float *);
		QByteArray _ConvertQV(const QVariant &v, QByteArray *);
		QString _ConvertQV(const QVariant &v, QString *);

		// 整数型 (4バイト以上)
		template <typename T>
			requires(std::integral<T> && sizeof(T) >= 4)
		T _ConvertQV(const QVariant &v, T *) {
			if constexpr (sizeof(T) == 8) {
				if constexpr (std::signed_integral<T>)
					return static_cast<T>(v.toLongLong());
				else
					return static_cast<T>(v.toULongLong());
			}
			else {
				if constexpr (std::signed_integral<T>)
					return static_cast<T>(v.toInt());
				else
					return static_cast<T>(v.toUInt());
			}
		}

		// enum class 型 (汎用対応)
		template <typename T>
			requires std::is_enum_v<T>
		T _ConvertQV(const QVariant &v, T *) {
			using Underlying = std::underlying_type_t<T>;
			Q_ASSERT(v.canConvert<Underlying>());
			return static_cast<T>(v.value<Underlying>());
		}

	} // namespace detail

	template <typename T>
	T ConvertQV(const QVariant &v) {
		if constexpr (std::is_enum_v<T>) {
			using Underlying = std::underlying_type_t<T>;
			Q_ASSERT(v.canConvert<Underlying>());
		}
		else {
			Q_ASSERT(v.canConvert<T>());
		}
		return detail::_ConvertQV(v, static_cast<T *>(nullptr));
	}
} // namespace dg
