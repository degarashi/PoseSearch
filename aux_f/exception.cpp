#include "exception.hpp"

namespace dg {
	// ------------------- RuntimeError -------------------
	RuntimeError::RuntimeError(const QString &msg) : _msg(msg) {
	}
	const QString &RuntimeError::q_what() const noexcept {
		return _msg;
	}

	// ------------------- InvalidInput -------------------
	InvalidInput::InvalidInput(const QString &reason) : RuntimeError(tr("invalid input: %1").arg(reason)) {
	}
	// ------------------- CantOpenFile -------------------
	CantOpenFile::CantOpenFile(const QString &path) : RuntimeError(tr("can't open file path(%1)").arg(path)) {
	}
	// ------------------- UnknownImage -------------------
	UnknownImage::UnknownImage(const QString &path) : RuntimeError(tr("can't load image (%1)").arg(path)) {
	}
	// ------------------- CantMakeThumbnail -------------------
	CantMakeThumbnail::CantMakeThumbnail(const QString &path) : RuntimeError(tr("can't make thumbnail for (%1)").arg(path)) {
	}
} // namespace dg
