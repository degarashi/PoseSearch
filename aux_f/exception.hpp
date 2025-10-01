#pragma once
#include <exception>
#include <string>

namespace dg {
	class RuntimeError : public std::exception {
		protected:
			std::string _msg; // エラーメッセージ格納

		public:
			explicit RuntimeError(const std::string &msg);

			// 標準的なwhat()をオーバーライド
			const char *what() const noexcept override;

			// 追加インターフェースとしてstd::string参照を返す
			virtual const std::string &s_what() const noexcept;
	};

	class InvalidInput : public RuntimeError {
		public:
			explicit InvalidInput(const std::string &reason);
	};

	class CantOpenFile : public RuntimeError {
		public:
			explicit CantOpenFile(const std::string &path);
	};

	class UnknownImage : public RuntimeError {
		public:
			explicit UnknownImage(const std::string &path);
	};

	class CantMakeThumbnail : public RuntimeError {
		public:
			explicit CantMakeThumbnail(const std::string &path);
	};
} // namespace dg
