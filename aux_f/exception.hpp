#pragma once
#include <QException>
#include <QObject>
#include <QString>

namespace dg {
	class RuntimeError : public QObject, public QException {
		Q_OBJECT
		protected:
			QString _msg;

		public:
			RuntimeError(const QString &msg);
			virtual const QString &q_what() const noexcept;
	};

	class InvalidInput : public RuntimeError {
		Q_OBJECT
		public:
			InvalidInput(const QString &reason);
	};
	class CantOpenFile : public RuntimeError {
		Q_OBJECT
		public:
			CantOpenFile(const QString &path);
	};
	class UnknownImage : public RuntimeError {
		Q_OBJECT
		public:
			UnknownImage(const QString &path);
	};
	class CantMakeThumbnail : public RuntimeError {
		Q_OBJECT
		public:
			CantMakeThumbnail(const QString &path);
	};
} // namespace dg
