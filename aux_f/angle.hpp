#pragma once
#include <cmath>
#include <compare>
#include <numbers>
#include <string>

namespace dg {
	// 前方宣言
	class Degree;
	class Radian;

	// 定数
	constexpr float DegPerCircle = 360.0f;
	constexpr float DegPerHalfCircle = DegPerCircle / 2.0f; // 180度
	constexpr float RadPerDeg = std::numbers::pi_v<float> / DegPerHalfCircle;
	constexpr float DegPerRad = DegPerHalfCircle / std::numbers::pi_v<float>;

	// constexpr で動く剰余計算
	constexpr float FMod(const float x, const float y) noexcept {
		return x - static_cast<int>(x / y) * y;
	}
	// -π〜π に正規化
	constexpr float NormalizeRadians(const float v) noexcept {
		const float twoPi = std::numbers::pi_v<float> * 2.0f;
		float r = FMod(v, twoPi);
		if (r <= -std::numbers::pi_v<float>)
			r += twoPi;
		else if (r > std::numbers::pi_v<float>)
			r -= twoPi;
		return r;
	}
	// -180〜180 に正規化
	constexpr float NormalizeDegrees(const float v) noexcept {
		float r = FMod(v, DegPerCircle);
		if (r <= -DegPerHalfCircle)
			r += DegPerCircle;
		else if (r > DegPerHalfCircle)
			r -= DegPerCircle;
		return r;
	}
	constexpr bool IsValidAngle(const float v) noexcept {
		// NaN と無限大を判定
		return !(std::isnan(v) || std::isinf(v));
	}

	// Radian型
	class Radian {
		private:
			float value;

			static constexpr float _sanitize(const float v) noexcept {
				return IsValidAngle(v) ? NormalizeRadians(v) : v;
			}

		public:
			explicit constexpr Radian() : value(0) {
			}
			explicit constexpr Radian(const float v) : value(_sanitize(v)) {
			}

			constexpr Radian(const Degree &deg);

			constexpr void set(const float v) noexcept {
				value = _sanitize(v);
			}
			constexpr void set(const Degree &deg) noexcept;
			constexpr float get() const noexcept {
				return value;
			}

			constexpr Degree toDegree() const noexcept;

			constexpr auto operator<=>(const Radian &other) const noexcept = default;
			constexpr bool operator==(const Radian &other) const noexcept = default;
			std::string toString() const {
				return std::to_string(value) + std::string("(rad)");
			}
	};

	// Degree型
	class Degree {
		private:
			float value;

			static constexpr float _sanitize(const float v) noexcept {
				return IsValidAngle(v) ? NormalizeDegrees(v) : v;
			}

		public:
			explicit constexpr Degree() : value(0) {
			}
			explicit constexpr Degree(const float v) : value(_sanitize(v)) {
			}

			constexpr Degree(const Radian &rad);

			constexpr void set(const float v) noexcept {
				value = _sanitize(v);
			}
			constexpr void set(const Radian &rad) noexcept;
			constexpr float get() const noexcept {
				return value;
			}

			constexpr Radian toRadian() const noexcept;

			constexpr auto operator<=>(const Degree &other) const noexcept = default;
			constexpr bool operator==(const Degree &other) const noexcept = default;
			std::string toString() const {
				return std::to_string(value) + std::string("(deg)");
			}
	};

	// 相互変換
	constexpr Radian::Radian(const Degree &deg) {
		const float dv = deg.get();
		value = IsValidAngle(dv) ? NormalizeRadians(dv * RadPerDeg) : dv;
	}
	constexpr Degree::Degree(const Radian &rad) {
		const float rv = rad.get();
		value = IsValidAngle(rv) ? NormalizeDegrees(rv * DegPerRad) : rv;
	}

	constexpr void Radian::set(const Degree &deg) noexcept {
		const float dv = deg.get();
		value = IsValidAngle(dv) ? NormalizeRadians(dv * RadPerDeg) : dv;
	}
	constexpr void Degree::set(const Radian &rad) noexcept {
		const float rv = rad.get();
		value = IsValidAngle(rv) ? NormalizeDegrees(rv * DegPerRad) : rv;
	}

	// 追加定義
	constexpr Degree Radian::toDegree() const noexcept {
		return Degree{*this};
	}
	constexpr Radian Degree::toRadian() const noexcept {
		return Radian{*this};
	}

	// 演算子オーバーロード（同型間）
	constexpr Radian operator+(const Radian &a, const Radian &b) {
		return Radian{a.get() + b.get()};
	}
	constexpr Radian operator-(const Radian &a, const Radian &b) {
		return Radian{a.get() - b.get()};
	}
	constexpr Degree operator+(const Degree &a, const Degree &b) {
		return Degree{a.get() + b.get()};
	}
	constexpr Degree operator-(const Degree &a, const Degree &b) {
		return Degree{a.get() - b.get()};
	}

	// スカラー倍
	template <std::floating_point T>
	constexpr Radian operator*(const Radian &a, const T s) {
		return Radian{a.get() * static_cast<float>(s)};
	}
	template <std::floating_point T>
	constexpr Radian operator*(const T s, const Radian &a) {
		return Radian{static_cast<float>(s) * a.get()};
	}
	template <std::floating_point T>
	constexpr Degree operator*(const Degree &a, const T s) {
		return Degree{a.get() * static_cast<float>(s)};
	}
	template <std::floating_point T>
	constexpr Degree operator*(const T s, const Degree &a) {
		return Degree{static_cast<float>(s) * a.get()};
	}

	// ユーザー定義リテラル
	constexpr Radian operator"" _rad(const long double v) {
		return Radian{static_cast<float>(v)};
	}
	constexpr Radian operator"" _rad(const unsigned long long v) {
		return Radian{static_cast<float>(v)};
	}
	constexpr Degree operator"" _deg(const long double v) {
		return Degree{static_cast<float>(v)};
	}
	constexpr Degree operator"" _deg(const unsigned long long v) {
		return Degree{static_cast<float>(v)};
	}
} // namespace dg
