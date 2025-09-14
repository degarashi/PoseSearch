#pragma once

#include <QByteArray>
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

} // namespace dg
