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
	constexpr double DegPerCircle = 360.0;
	constexpr double DegPerHalfCircle = DegPerCircle / 2.0; // 180度
	constexpr double RadPerDeg = std::numbers::pi / DegPerHalfCircle;
	constexpr double DegPerRad = DegPerHalfCircle / std::numbers::pi;

	// 値の正規化ヘルパー
	constexpr double NormalizeRadians(const double v) noexcept {
		// -π〜π に正規化
		const double twoPi = std::numbers::pi * 2.0;
		double r = std::fmod(v, twoPi);
		if (r <= -std::numbers::pi)
			r += twoPi;
		else if (r > std::numbers::pi)
			r -= twoPi;
		return r;
	}
	constexpr double NormalizeDegrees(const double v) noexcept {
		// -180〜180 に正規化
		double r = std::fmod(v, DegPerCircle);
		if (r <= -DegPerHalfCircle)
			r += DegPerCircle;
		else if (r > DegPerHalfCircle)
			r -= DegPerCircle;
		return r;
	}
	constexpr bool IsValidAngle(const double v) noexcept {
		return std::isfinite(v);
	}

	// Radian型
	class Radian {
		private:
			double value;

			static constexpr double _sanitize(const double v) noexcept {
				return IsValidAngle(v) ? NormalizeRadians(v) : 0.0;
			}

		public:
			explicit constexpr Radian() : value(0) {
			}
			explicit constexpr Radian(const double v) : value(_sanitize(v)) {
			}

			constexpr Radian(const Degree &deg);

			constexpr double toDouble() const noexcept {
				return value;
			}

			constexpr void set(const double v) noexcept {
				value = _sanitize(v);
			}
			constexpr void set(const Degree &deg) noexcept;
			constexpr double get() const noexcept {
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
			double value;

			static constexpr double _sanitize(const double v) noexcept {
				return IsValidAngle(v) ? NormalizeDegrees(v) : 0.0;
			}

		public:
			explicit constexpr Degree() : value(0) {
			}
			explicit constexpr Degree(const double v) : value(_sanitize(v)) {
			}

			constexpr Degree(const Radian &rad);

			constexpr double toDouble() const noexcept {
				return value;
			}

			constexpr void set(const double v) noexcept {
				value = _sanitize(v);
			}
			constexpr void set(const Radian &rad) noexcept;
			constexpr double get() const noexcept {
				return value;
			}

			constexpr auto operator<=>(const Degree &other) const noexcept = default;
			constexpr bool operator==(const Degree &other) const noexcept = default;
			QString toString() const {
				return QString::number(value) + QStringLiteral("(deg)");
			}
	};

	// 相互変換
	constexpr Radian::Radian(const Degree &deg) : value(_sanitize(deg.toDouble() * RadPerDeg)) {
	}
	constexpr Degree::Degree(const Radian &rad) : value(_sanitize(rad.toDouble() * DegPerRad)) {
	}

	constexpr void Radian::set(const Degree &deg) noexcept {
		value = _sanitize(deg.toDouble() * RadPerDeg);
	}
	constexpr void Degree::set(const Radian &rad) noexcept {
		value = _sanitize(rad.toDouble() * DegPerRad);
	}

	// 演算子オーバーロード（同型間）
	constexpr Radian operator+(const Radian &a, const Radian &b) {
		return Radian{a.toDouble() + b.toDouble()};
	}
	constexpr Radian operator-(const Radian &a, const Radian &b) {
		return Radian{a.toDouble() - b.toDouble()};
	}
	constexpr Degree operator+(const Degree &a, const Degree &b) {
		return Degree{a.toDouble() + b.toDouble()};
	}
	constexpr Degree operator-(const Degree &a, const Degree &b) {
		return Degree{a.toDouble() - b.toDouble()};
	}

	// スカラー倍
	template <std::floating_point T>
	constexpr Radian operator*(const Radian &a, const T s) {
		return Radian{a.toDouble() * static_cast<double>(s)};
	}
	template <std::floating_point T>
	constexpr Radian operator*(const T s, const Radian &a) {
		return Radian{static_cast<double>(s) * a.toDouble()};
	}
	template <std::floating_point T>
	constexpr Degree operator*(const Degree &a, const T s) {
		return Degree{a.toDouble() * static_cast<double>(s)};
	}
	template <std::floating_point T>
	constexpr Degree operator*(const T s, const Degree &a) {
		return Degree{static_cast<double>(s) * a.toDouble()};
	}

	// ユーザー定義リテラル
	constexpr Radian operator"" _rad(const long double v) {
		return Radian{static_cast<double>(v)};
	}
	constexpr Radian operator"" _rad(const unsigned long long v) {
		return Radian{static_cast<double>(v)};
	}
	constexpr Degree operator"" _deg(const long double v) {
		return Degree{static_cast<double>(v)};
	}
	constexpr Degree operator"" _deg(const unsigned long long v) {
		return Degree{static_cast<double>(v)};
	}
} // namespace dg
