#pragma once

#include <QByteArray>
#include <QStringList>
#include <QVector2D>
#include <QVector3D>
#include <QVector4D>
#include <tuple>
#include <type_traits>

namespace dg {

	template <typename T, typename Tuple>
	struct tuple_index;

	template <typename T, typename... Types>
	struct tuple_index<T, std::tuple<T, Types...>> : std::integral_constant<std::size_t, 0> {};

	template <typename T, typename U, typename... Types>
	struct tuple_index<T, std::tuple<U, Types...>>
		: std::integral_constant<std::size_t, 1 + tuple_index<T, std::tuple<Types...>>::value> {};

	namespace {
		using TypeList = std::tuple<float, QVector2D, QVector3D, QVector4D>;
	}

	// ----------- VecToByteArray -----------
	template <typename Src_t, size_t... Idx>
	QByteArray _VecToByteArray(const Src_t &v, std::index_sequence<Idx...>) {
		QByteArray ret;
		const float va[sizeof...(Idx)] = {v[Idx]...};
		ret.append(reinterpret_cast<const char *>(va), sizeof(va));
		return std::move(ret);
	}

	template <typename Src_t>
	QByteArray VecToByteArray(const Src_t &v) {
		constexpr size_t Dim = tuple_index<Src_t, TypeList>::value + 1;
		return _VecToByteArray(v, std::make_index_sequence<Dim>{});
	}

	// ----------- ByteArrayToVec -----------
	template <class T, size_t N, size_t... Idx>
	T _MakeReturn(float (&data)[N], std::index_sequence<Idx...>) {
		static_assert(N == sizeof...(Idx));
		return T{data[Idx]...};
	}
	template <typename Ret_t, typename Src_t, size_t N>
	auto MakeReturn(Src_t (&data)[N]) {
		return _MakeReturn<Ret_t>(data, std::make_index_sequence<N>{});
	}

	template <size_t N>
	auto ByteArrayToVec(const QByteArray &ba) {
		using Ret = std::tuple_element_t<N - 1, TypeList>;
		Q_ASSERT(ba.size() == sizeof(Ret));
		float va[N];
		memcpy(va, ba.constData(), sizeof(va));
		return MakeReturn<Ret>(va);
	}

	// ----------- VecToString -----------
	// メンバ関数の存在を判定する concept
	template <typename T>
	concept HasW = requires(const T &t) {
		{ t.w() } -> std::convertible_to<float>;
	};

	template <typename T>
	concept HasZ = requires(const T &t) {
		{ t.z() } -> std::convertible_to<float>;
	};

	// 次元数をコンパイル時に決定
	template <typename T>
	constexpr int vec_dimension() {
		if constexpr (HasW<T>) {
			return 4;
		}
		else if constexpr (HasZ<T>) {
			return 3;
		}
		else {
			return 2;
		}
	}

	// 本体
	template <typename Vec>
		requires requires(const Vec &v) {
			{ v[0] } -> std::convertible_to<float>;
		}
	QString VecToString(const Vec &v, int precision = 3) {
		QStringList parts;
		constexpr int N = vec_dimension<Vec>();
		for (int i = 0; i < N; ++i) {
			parts << QString::number(v[i], 'f', precision);
		}
		return "[" + parts.join(' ') + "]";
	}
} // namespace dg
