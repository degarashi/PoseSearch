#pragma once
#include "getvalue.hpp"
#include <tuple>

namespace dg::sql {
	namespace detail {
		template <class Q>
		auto GetRequiredTuple(Q&&, size_t, std::tuple<>*) {
			return std::tuple<>{};
		}
		template <class Q, class T0, class... Ts>
		auto GetRequiredTuple(Q&& q, const size_t columnId, std::tuple<T0, Ts...>*) {
			return std::tuple_cat(
				std::tuple<T0>{
					GetRequiredValue<T0>(q, columnId, false)
				},
				GetRequiredTuple(
					q,
					columnId+1,
					static_cast<std::tuple<Ts...>*>(nullptr)
				)
			);
		}
	}
	template <class... Ts, class Q>
	auto GetRequiredTuple(Q&& q, const size_t columnId=0, const bool advance=true) {
		if(advance) {
			if(!q.next())
				throw std::runtime_error("value not found");
		}
		return detail::GetRequiredTuple(
			q,
			columnId,
			static_cast<std::tuple<Ts...>*>(nullptr)
		);
	}
}
