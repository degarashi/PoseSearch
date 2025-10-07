#pragma once

#include <array>
#include <format>
#include <source_location>
#include <type_traits>
#include <utility>
#include <vector>

namespace dg {
#if !defined(NDEBUG)
	class WithLocationUtil {
		public:
			inline static thread_local std::vector<std::source_location> tls_location;

			static void PushLocation(const std::source_location loc) {
				tls_location.push_back(loc);
			}
			static void PopLocation() {
				tls_location.pop_back();
			}

			static std::string OutputSourceLog() {
				std::string ret;
				for (auto &loc : tls_location) {
					ret += LocationStr(loc);
					ret += "\n";
				}
				return ret;
			}
			static std::string LocationStr(const std::source_location &loc) {
				return std::format("[{}({} : {})]", loc.file_name(), loc.line(), loc.column());
			}
	};

	template <typename T>
	class WithLocationWrap {
		private:
			T _value;

			static_assert(!std::is_reference_v<T>, "WithLocationWrap<T>: T must not be a reference type");

		public:
			template <typename TA>
			explicit(!std::is_convertible_v<TA, T>)
				WithLocationWrap(TA &&value, std::source_location loc = std::source_location::current()) :
				_value(std::forward<TA>(value)) {
				WithLocationUtil::PushLocation(loc);
			}
			~WithLocationWrap() {
				WithLocationUtil::PopLocation();
			}
			operator const T &() const noexcept {
				return _value;
			}
	};

	// CTAD
	template <typename TA>
	WithLocationWrap(TA &&) -> WithLocationWrap<std::decay_t<TA>>;
	template <typename T, std::size_t N>
	WithLocationWrap(T (&)[N]) -> WithLocationWrap<std::array<T, N>>;

	template <typename T>
	using WithLocation = WithLocationWrap<T>;

#else

	class WithLocationUtil {
		public:
			static void PushLocation(const std::source_location loc) {
			}
			static void PopLocation() {
			}
			static std::string OutputSourceLog() {
				return {};
			}
	};

	template <typename T>
	using WithLocation = T;

#endif

} // namespace dg
