#pragma once

#include "color.hpp"
#include <mutex>
#include <optional>
#include <vector>
namespace squi {
	struct Theme {
		Color accent = 0x60CDFFFF;
	};

	struct ThemeManager {
		static std::optional<Color> getSystemAccentColor();

		struct ThemePusher {
			ThemePusher() = default;
			~ThemePusher() {
				std::scoped_lock _{_mtx};
				stack.pop_back();
			}
		};

		static inline Theme getTheme() {
			std::scoped_lock _{_mtx};
			return stack.back();
		}

		[[nodiscard]] static inline ThemePusher pushTheme(const Theme &theme) {
			std::scoped_lock _{_mtx};
			stack.emplace_back(theme);
			return ThemePusher{};
		}

	private:
		static inline std::mutex _mtx;
		static inline std::vector<Theme> stack{Theme{}};
	};

}// namespace squi