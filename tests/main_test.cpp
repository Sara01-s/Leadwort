#include <gtest/gtest.h>

TEST(EngineTest, CanaryTest) {
	constexpr int a = 1;
	constexpr int b = 2;

	EXPECT_EQ(a + b, 3);

	std::vector miVector = { 10, 20, 30, 40 };
}

int main(int argc, char **argv) {
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}