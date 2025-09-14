#pragma once
#include "aux_f/exception.hpp"

namespace dg {
	/**
	 * @brief 値を別の範囲にマッピング(線形補間)
	 *
	 * @tparam T 値の型
	 * @param inVal マッピングする入力値
	 * @param inMin 入力値の最小値
	 * @param inMax 入力値の最大値
	 * @param outMin 出力値の最小値
	 * @param outMax 出力値の最大値
	 * @return マッピングされた出力値
	 */
	template <typename T>
	T Remap(const T &inVal, const T &inMin, const T &inMax, const T &outMin, const T &outMax) {
		// ゼロ除算を防ぐ
		if (inMin == inMax)
			return (inVal == inMin) ? outMin : outMax;
		return ((inVal - inMin) / (inMax - inMin)) * (outMax - outMin) + outMin;
	}

	template <typename T>
	struct Range {
			T min, max;

			constexpr Range() = default;
			constexpr Range(const T &min_v, const T &max_v) noexcept : min(min_v), max(max_v) {
			}

			void validate() const {
				if (!valid())
					throw dg::InvalidInput(QString("invalid range min=%1, max=%2").arg(min, max));
			}
			constexpr bool valid() const noexcept {
				return min <= max;
			}
	};
} // namespace dg
