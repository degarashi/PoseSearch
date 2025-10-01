cmake_minimum_required(VERSION 3.16)
project(PoseSearch VERSION 0.1 LANGUAGES CXX)
include(ExternalProject)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# -----------------------------------
include(FetchContent)

# GoogleTest を取得
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.17.0
  GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(googletest)
enable_testing()
add_executable(mytests
	tests/test_angle.cpp
	tests/test_value.cpp
	aux_f/exception.cpp
)

target_include_directories(mytests PRIVATE . aux_f)
target_link_libraries(mytests
	PRIVATE
		GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(mytests)
# -----------------------------------

# compile_commands.json をツール用にコピー
add_custom_target(copy-compile-commands ALL
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
	${CMAKE_BINARY_DIR}/compile_commands.json
	${CMAKE_SOURCE_DIR}/compile_commands.json
)

# ビルドタイプに応じた最適化フラグの設定
set(OPTIMIZATION_FLAGS "-O2")
if(CMAKE_BUILD_TYPE MATCHES "Debug")
	set(OPTIMIZATION_FLAGS "-O0")
endif()

# sqlite-vec.dll をビルド
ExternalProject_Add(sqlite_vec
	SOURCE_DIR ${CMAKE_SOURCE_DIR}/sqlite3
	CONFIGURE_COMMAND ""
	BUILD_COMMAND clang ${OPTIMIZATION_FLAGS} -shared -o ${CMAKE_BINARY_DIR}/sqlite-vec.dll ${CMAKE_SOURCE_DIR}/sqlite3/sqlite-vec.c -I. -fPIC
	INSTALL_COMMAND ""
	BUILD_BYPRODUCTS ${CMAKE_BINARY_DIR}/sqlite-vec.dll
)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED
	COMPONENTS
	Widgets
	Sql
	StateMachine
	Concurrent
)

# ファイル検索対象のディレクトリリスト
set(TARGET_FILES_DIR
	.
	aux_f
	aux_f/sql
	param
	widget
	singleton
	condition
)

# @brief 指定されたディレクトリからソースファイルとヘッダーファイルを収集する関数
#
# @param target_dirs 検索対象のディレクトリリスト
# @param out_headers 収集されたヘッダーファイルのリストを格納する変数名
# @param out_sources 収集されたソースファイルのリストを格納する変数名
function(collect_source_and_headers target_dirs out_headers out_sources)
	set(collected_headers)
	set(collected_sources)
	foreach(dir ${target_dirs})
		file(GLOB TMP_HEADER RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${dir}/*.h ${dir}/*.hpp ${dir}/*.ui)
		aux_source_directory(${dir} TMP_SRC)
		list(APPEND collected_headers ${TMP_HEADER})
		list(APPEND collected_sources ${TMP_SRC})
	endforeach()
	set(${out_headers} ${collected_headers} PARENT_SCOPE)
	set(${out_sources} ${collected_sources} PARENT_SCOPE)
endfunction()

collect_source_and_headers("${TARGET_FILES_DIR}" MY_HEADERS MY_SOURCES)

qt_add_executable(PoseSearch
	MANUAL_FINALIZATION
	${MY_SOURCES}
	${MY_HEADERS}
	widget/poseinfodialog.h widget/poseinfodialog.cpp widget/poseinfodialog.ui
)
add_dependencies(PoseSearch sqlite_vec)

target_sources(PoseSearch PRIVATE ${MY_SOURCES})
target_include_directories(PoseSearch PRIVATE . cereal cereal_types)
target_link_libraries(PoseSearch
	PRIVATE
	Qt${QT_VERSION_MAJOR}::Widgets
	Qt${QT_VERSION_MAJOR}::Sql
	Qt${QT_VERSION_MAJOR}::StateMachine
	Qt${QT_VERSION_MAJOR}::Concurrent
)

set_target_properties(PoseSearch PROPERTIES
	${BUNDLE_ID_OPTION}
	MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
	MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
	MACOSX_BUNDLE TRUE
	WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS PoseSearch
	BUNDLE DESTINATION .
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

qt_finalize_executable(PoseSearch)
