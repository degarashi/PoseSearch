#pragma once
#include "gettuple.hpp"
#include <vector>

namespace dg::sql {
	template <class T, class Q>
	auto GetValues(Q&& q, const size_t columnId=0) {
		std::vector<T> ret;
		while(q.next()) {
			ret.emplace_back(GetRequiredValue<T>(q, columnId, false));
		}
		return ret;
	}
	template <class... Ts, class Q>
	auto GetTupleValues(Q&& q, const size_t columnId=0) {
		std::vector<std::tuple<Ts...>> ret;
		while(q.next()) {
			ret.emplace_back(GetRequiredTuple<Ts...>(q, columnId, false));
		}
		return ret;
	}
}
