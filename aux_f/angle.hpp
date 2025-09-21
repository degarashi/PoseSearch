#pragma once
#include <QString>
#include <cmath>
#include <compare>
#include <numbers>

namespace dg {
	// 前方宣言
	class Degree;
	class Radian;

	// 定数
	constexpr float DegPerCircle = 360.0f;
	constexpr float DegPerHalfCircle = DegPerCircle / 2.0f; // 180度
	constexpr float RadPerDeg = std::numbers::pi_v<float> / DegPerHalfCircle;
	constexpr float DegPerRad = DegPerHalfCircle / std::numbers::pi_v<float>;

	// 値の正規化ヘルパー
	constexpr float NormalizeRadians(const float v) noexcept {
		// -π〜π に正規化
		const float twoPi = std::numbers::pi_v<float> * 2.0f;
		float r = std::fmod(v, twoPi);
		if (r <= -std::numbers::pi_v<float>)
			r += twoPi;
		else if (r > std::numbers::pi_v<float>)
			r -= twoPi;
		return r;
	}
	constexpr float NormalizeDegrees(const float v) noexcept {
		// -180〜180 に正規化
		float r = std::fmod(v, DegPerCircle);
		if (r <= -DegPerHalfCircle)
			r += DegPerCircle;
		else if (r > DegPerHalfCircle)
			r -= DegPerCircle;
		return r;
	}
	constexpr bool IsValidAngle(const float v) noexcept {
		return std::isfinite(v);
	}

	// Radian型
	class Radian {
		private:
			float value;

			static constexpr float _sanitize(const float v) noexcept {
				return IsValidAngle(v) ? NormalizeRadians(v) : 0.0f;
			}

		public:
			explicit constexpr Radian() : value(0) {
			}
			explicit constexpr Radian(const float v) : value(_sanitize(v)) {
			}

			constexpr Radian(const Degree &deg);

			constexpr float toFloat() const noexcept {
				return value;
			}

			constexpr void set(const float v) noexcept {
				value = _sanitize(v);
			}
			constexpr void set(const Degree &deg) noexcept;
			constexpr float get() const noexcept {
				return value;
			}

			constexpr auto operator<=>(const Radian &other) const noexcept = default;
			constexpr bool operator==(const Radian &other) const noexcept = default;
			QString toString() const {
				return QString::number(value) + QStringLiteral("(rad)");
			}
	};

	// Degree型
	class Degree {
		private:
			float value;

			static constexpr float _sanitize(const float v) noexcept {
				return IsValidAngle(v) ? NormalizeDegrees(v) : 0.0f;
			}

		public:
			explicit constexpr Degree() : value(0) {
			}
			explicit constexpr Degree(const float v) : value(_sanitize(v)) {
			}

			constexpr Degree(const Radian &rad);

			constexpr float toFloat() const noexcept {
				return value;
			}

			constexpr void set(const float v) noexcept {
				value = _sanitize(v);
			}
			constexpr void set(const Radian &rad) noexcept;
			constexpr float get() const noexcept {
				return value;
			}

			constexpr auto operator<=>(const Degree &other) const noexcept = default;
			constexpr bool operator==(const Degree &other) const noexcept = default;
			QString toString() const {
				return QString::number(value) + QStringLiteral("(deg)");
			}
	};

	// 相互変換
	constexpr Radian::Radian(const Degree &deg) : value(_sanitize(deg.toFloat() * RadPerDeg)) {
	}
	constexpr Degree::Degree(const Radian &rad) : value(_sanitize(rad.toFloat() * DegPerRad)) {
	}

	constexpr void Radian::set(const Degree &deg) noexcept {
		value = _sanitize(deg.toFloat() * RadPerDeg);
	}
	constexpr void Degree::set(const Radian &rad) noexcept {
		value = _sanitize(rad.toFloat() * DegPerRad);
	}

	// 演算子オーバーロード（同型間）
	constexpr Radian operator+(const Radian &a, const Radian &b) {
		return Radian{a.toFloat() + b.toFloat()};
	}
	constexpr Radian operator-(const Radian &a, const Radian &b) {
		return Radian{a.toFloat() - b.toFloat()};
	}
	constexpr Degree operator+(const Degree &a, const Degree &b) {
		return Degree{a.toFloat() + b.toFloat()};
	}
	constexpr Degree operator-(const Degree &a, const Degree &b) {
		return Degree{a.toFloat() - b.toFloat()};
	}

	// スカラー倍
	template <std::floating_point T>
	constexpr Radian operator*(const Radian &a, const T s) {
		return Radian{a.toFloat() * static_cast<float>(s)};
	}
	template <std::floating_point T>
	constexpr Radian operator*(const T s, const Radian &a) {
		return Radian{static_cast<float>(s) * a.toFloat()};
	}
	template <std::floating_point T>
	constexpr Degree operator*(const Degree &a, const T s) {
		return Degree{a.toFloat() * static_cast<float>(s)};
	}
	template <std::floating_point T>
	constexpr Degree operator*(const T s, const Degree &a) {
		return Degree{static_cast<float>(s) * a.toFloat()};
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
