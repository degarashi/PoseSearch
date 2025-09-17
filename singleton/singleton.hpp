#pragma once
#include <cassert>
#include <memory>

namespace dg {
	using DeinitF = void (*)();
	class Deinit {
		private:
			DeinitF _func;

		public:
			explicit Deinit(DeinitF func) : _func(func) {
			}
			Deinit(const Deinit &) = delete;

			~Deinit() {
				_func();
			}
	};
	template <typename T>
	class Singleton {
		private:
			using s_type = T;
		public:
			static s_type &Get() {
				assert(_instance);
				return *_instance;
			}
			static const s_type &GetC() {
				return Get();
			}
			template <typename... Ts>
			static void InitializeUsing(Ts &&...ts) {
				Initialize(new s_type(std::forward<Ts>(ts)...));
			}
			static void Initialize(s_type *instance) {
				assert(!_instance);
				_instance.reset(instance);
			}
			static void Destroy() {
				assert(_instance);
				_instance.reset();
			}
			[[nodiscard]] static Deinit ScopeDeinit() {
				assert(!_make_deinit);
				_make_deinit = true;
				return Deinit{&Destroy};
			}

		private:
			inline static std::unique_ptr<s_type> _instance;
			inline static bool _make_deinit = false;

		protected:
			Singleton() = default;
	};
} // namespace dg
