include(FetchContent)

FetchContent_Declare(
	googletest
	GIT_REPOSITORY https://github.com/google/googletest.git
	GIT_TAG v1.17.0
	GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(mytests
	test_angle.cpp
	test_value.cpp
)

# 共通ライブラリをリンク
target_link_libraries(mytests
	PRIVATE
	PoseSearchLib
	GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(mytests)
