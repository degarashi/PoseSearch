#include <gtest/gtest.h>
#include "value.hpp"

using namespace dg;

// -------- Remap テスト --------

// 【通常のマッピングテスト】
// 入力値を [inMin, inMax] → [outMin, outMax] に線形変換する基本動作を確認
TEST(RemapTest, NormalMapping) {
	// 5.0 を [0.0, 10.0] → [0.0, 100.0] にマッピング → 50.0 になるはず
	EXPECT_DOUBLE_EQ(Remap(5.0, 0.0, 10.0, 0.0, 100.0), 50.0);
	// 整数版: 2 を [0, 4] → [0, 8] にマッピング → 4 になるはず
	EXPECT_EQ(Remap(2, 0, 4, 0, 8), 4);
}

// 【出力範囲が逆転している場合のテスト】
// 出力範囲が [100.0, 0.0] のように逆順でも正しく補間されるか確認
TEST(RemapTest, ReverseOutputRange) {
	// 5.0 を [0.0, 10.0] → [100.0, 0.0] にマッピング → 50.0 になるはず
	EXPECT_DOUBLE_EQ(Remap(5.0, 0.0, 10.0, 100.0, 0.0), 50.0);
}

// 【入力値が inMin と等しい場合のテスト】
// 入力が inMin の場合、出力は outMin になるはず
TEST(RemapTest, InputEqualsMin) {
	EXPECT_EQ(Remap(0, 0, 10, 100, 200), 100);
}

// 【入力値が inMax と等しい場合のテスト】
// 入力が inMax の場合、出力は outMax になるはず
// inMin == inMax の特殊ケースも含む
TEST(RemapTest, InputEqualsMax) {
	EXPECT_EQ(Remap(10, 0, 10, 100, 200), 200);
}

// 【入力範囲が退化している場合のテスト】
// inMin == inMax の場合の挙動を確認
TEST(RemapTest, DegenerateRange) {
	// inMin == inMax の場合、仕様上 outMax を返す
	EXPECT_EQ(Remap(5, 5, 5, 10, 20), 20);
	// outMin == outMax の場合、常にその値を返す
	EXPECT_EQ(Remap(5, 5, 5, 10, 10), 10);
}

// -------- Range テスト --------

// 【正常な範囲のテスト】
// min <= max の場合、valid() が true、validate() が例外を投げないことを確認
TEST(RangeTest, ValidRange) {
	Range<int> r(1, 5);
	EXPECT_TRUE(r.valid());
	EXPECT_NO_THROW(r.validate());
}

// 【不正な範囲のテスト】
// min > max の場合、valid() が false、validate() が例外を投げることを確認
TEST(RangeTest, InvalidRange) {
	Range<int> r(10, 5);
	EXPECT_FALSE(r.valid());
	EXPECT_THROW(r.validate(), dg::InvalidInput);
}

// 【デフォルトコンストラクタのテスト】
// デフォルト構築時は未初期化の可能性があるため、valid() の結果は未定義
// ここでは単に呼び出しが可能であることを確認
TEST(RangeTest, DefaultConstructor) {
	Range<int> r;
	r.valid();
}
