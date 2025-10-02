#pragma once
#include <vector>

enum class FileId : int {};
using FileIds = std::vector<FileId>;

enum class PoseId : int {};
using PoseIds = std::vector<PoseId>;

template <typename T>
auto EnumToInt(const T &t) {
	return std::underlying_type_t<T>(t);
}
