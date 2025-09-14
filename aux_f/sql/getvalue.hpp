#pragma once
#include "value.hpp"
#include <exception>

namespace dg::sql {
	template <class T, class Q>
	std::optional<T> GetValue(Q&& q, const size_t columnId=0, const bool advance=true) {
		if(!advance || q.next()) {
			const QVariant val = q.value(columnId);
			if(!val.isNull())
				return ConvertQV<T>(val);
		}
		return std::nullopt;
	}
	template <class T, class Q>
	T GetRequiredValue(Q&& q, const size_t columnId=0, const bool advance=true) {
		if(const auto ret = GetValue<T>(q, columnId, advance))
			return *ret;
		throw std::runtime_error("value not found");
	}
}
