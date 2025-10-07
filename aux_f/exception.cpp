#include "exception.hpp"
#include "aux_f/debug.hpp"

namespace dg {
	namespace {
		std::string MakeString(const std::string& msg) {
			return WithLocationUtil::OutputSourceLog() + msg;
		}
	} // namespace
	// ------------------- RuntimeError -------------------
	RuntimeError::RuntimeError(const std::string &msg) : _msg(MakeString(msg)) {
	}

	const char *RuntimeError::what() const noexcept {
		return s_what().c_str();
	}

	const std::string &RuntimeError::s_what() const noexcept {
		return _msg;
	}

	// ------------------- InvalidInput -------------------
	InvalidInput::InvalidInput(const std::string &reason) : RuntimeError("invalid input: " + reason) {
	}

	// ------------------- CantOpenFile -------------------
	CantOpenFile::CantOpenFile(const std::string &path) : RuntimeError("can't open file path(" + path + ")") {
	}

	// ------------------- UnknownImage -------------------
	UnknownImage::UnknownImage(const std::string &path) : RuntimeError("can't load image (" + path + ")") {
	}

	// ------------------- CantMakeThumbnail -------------------
	CantMakeThumbnail::CantMakeThumbnail(const std::string &path) :
		RuntimeError("can't make thumbnail for (" + path + ")") {
	}
} // namespace dg
