#pragma once
#include <format>
#include <type_traits>
#include "aux_f/exception.hpp"

namespace dg {
	/**
	 * @brief 値を別の範囲にマッピング(線形補間)
	 *
	 * @tparam T 値の型 (算術型に制約)
	 * @param inVal マッピングする入力値
	 * @param inMin 入力値の最小値
	 * @param inMax 入力値の最大値
	 * @param outMin 出力値の最小値
	 * @param outMax 出力値の最大値
	 * @return マッピングされた出力値
	 */
	template <typename T>
		requires std::is_arithmetic_v<T>
	T Remap(const T &inVal, const T &inMin, const T &inMax, const T &outMin, const T &outMax) {
		// ゼロ除算を防ぐ
		if (inMin == inMax) {
			// inMin==inMax の場合は outMax を返す
			return outMax;
		}
		// 浮動小数点にキャストして精度を確保
		using F = std::conditional_t<std::is_floating_point_v<T>, T, double>;
		const F ratio = static_cast<F>(inVal - inMin) / static_cast<F>(inMax - inMin);
		return static_cast<T>(ratio * static_cast<F>(outMax - outMin) + static_cast<F>(outMin));
	}

	template <typename T>
	struct Range {
			T min, max;

			constexpr Range() = default;
			constexpr Range(const T &min_v, const T &max_v) noexcept : min(min_v), max(max_v) {
			}

			void validate() const {
				if (!valid()) {
					if constexpr (std::is_arithmetic_v<T>)
						throw dg::InvalidInput(std::format("invalid range min={}, max={}", min, max));
					else
						throw dg::InvalidInput("invalid range (non-arithmetic type)");
				}
			}
			constexpr bool valid() const noexcept {
				if constexpr (std::is_floating_point_v<T>) {
					// NaN を考慮
					if (std::isnan(min) || std::isnan(max))
						return false;
				}
				return min <= max;
			}
	};
} // namespace dg
