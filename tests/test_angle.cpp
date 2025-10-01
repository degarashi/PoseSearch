#include <gtest/gtest.h>
#include <numbers>
#include "aux_f/angle.hpp"

using namespace dg;

// Radian正規化処理のテスト
// 入力値を -π〜π の範囲に収める動作確認
TEST(AngleTest, NormalizeRadians) {
	constexpr auto PI = std::numbers::pi_v<float>;
	EXPECT_NEAR(NormalizeRadians(0.0f), 0.0f, 1e-6); // 0の正規化確認
	EXPECT_NEAR(NormalizeRadians(-PI), PI, 1e-6);
	EXPECT_NEAR(NormalizeRadians(PI), PI, 1e-6);
	EXPECT_NEAR(NormalizeRadians(PI * 2), 0.f, 1e-6); // 2πの正規化確認
	EXPECT_NEAR(NormalizeRadians(-PI * 3 + 1e-4), -PI + 1e-4,
				1e-6); // -3π+微小値の正規化確認
}

// Degree正規化処理のテスト
// 入力値を -180〜180 の範囲に収める動作確認
TEST(AngleTest, NormalizeDegrees) {
	EXPECT_NEAR(NormalizeDegrees(0.0f), 0.0f, 1e-6); // 0の正規化確認
	EXPECT_NEAR(NormalizeDegrees(-180.f), 180.0f, 1e-6);
	EXPECT_NEAR(NormalizeDegrees(180.f), 180.f, 1e-6);
	EXPECT_NEAR(NormalizeDegrees(540.0f - 1.f), 180.0f - 1.f, 1e-6); // 540-1度の正規化確認
	EXPECT_NEAR(NormalizeDegrees(-540.0f + 1.f), -180.0f + 1.f, 1e-6); // -540+1度の正規化確認
}

// DegreeからRadianへの変換と逆変換のテスト
// 180度がπラジアンに変換されることを確認し、再度Degreeに戻すと180度になることを確認
TEST(AngleTest, DegreeToRadianAndBack) {
	Degree d{180.0f};
	Radian r = d.toRadian();
	EXPECT_NEAR(r.get(), std::numbers::pi_v<float>, 1e-6);

	Degree d2 = r.toDegree();
	EXPECT_NEAR(d2.get(), 180.0f, 1e-6);
}

// RadianからDegreeへの変換と逆変換のテスト
// π/2ラジアンが90度に変換されることを確認し、再度Radianに戻すとπ/2ラジアンになることを確認
TEST(AngleTest, RadianToDegreeAndBack) {
	Radian r{std::numbers::pi_v<float> / 2.0f};
	Degree d = r.toDegree();
	EXPECT_NEAR(d.get(), 90.0f, 1e-6);

	Radian r2 = d.toRadian();
	EXPECT_NEAR(r2.get(), std::numbers::pi_v<float> / 2.0f, 1e-6);
}

// 加算と減算演算子のテスト
// Degree同士およびRadian同士の加算結果が正しく正規化されることを確認
TEST(AngleTest, OperatorsAddSub) {
	Degree d1{90.0f};
	Degree d2{100.0f};
	Degree d3 = d1 + d2;
	EXPECT_NEAR(d3.get(), -170.0f, 1e-6); // 190度が正規化されて-170度になることを確認

	Radian r1{std::numbers::pi_v<float> / 2.0f};
	Radian r2{std::numbers::pi_v<float> / 2.0f};
	Radian r3 = r1 + r2;
	EXPECT_NEAR(r3.get(), std::numbers::pi_v<float>, 1e-6); // π/2 + π/2 = π の確認
}

// スカラー倍演算子のテスト
// DegreeおよびRadianに対してスカラー倍が正しく適用されることを確認
TEST(AngleTest, OperatorsScalarMul) {
	Degree d{89.0f};
	Degree d2 = d * 2.0f;
	EXPECT_NEAR(d2.get(), 178.0f, 1e-6); // 89度×2 = 178度の確認

	Radian r{std::numbers::pi_v<float> / 4.0f};
	Radian r2 = 2.0f * r;
	EXPECT_NEAR(r2.get(), std::numbers::pi_v<float> / 2.0f, 1e-6); // π/4×2 = π/2 の確認
}

// ユーザー定義リテラルのテスト
// _deg および _rad リテラルが正しくDegreeとRadianを生成することを確認
TEST(AngleTest, UserDefinedLiterals) {
	auto d = 180_deg;
	auto r = 3.14159265358979323846_rad;

	EXPECT_NEAR(d.get(), 180.0f, 1e-6); // 180度リテラルの確認
	EXPECT_NEAR(r.get(), std::numbers::pi_v<float>, 1e-6); // πラジアンリテラルの確認
}

// 無効値判定のテスト
// 無限大やNaNが無効と判定され、通常の数値は有効と判定されることを確認
TEST(AngleTest, InvalidValues) {
	EXPECT_FALSE(IsValidAngle(std::numeric_limits<float>::infinity())); // 無限大の確認
	EXPECT_FALSE(IsValidAngle(std::numeric_limits<float>::quiet_NaN())); // NaNの確認
	EXPECT_TRUE(IsValidAngle(123.45f)); // 通常値の確認
}
