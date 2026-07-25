#pragma once

#include "GLFW/glfw3.h"
#include "core/app.hpp"


namespace squi::core {
	struct Clipboard {
		App *app;

		[[nodiscard]] inline std::string get() const {
			return glfwGetClipboardString(app->engine.instance.window.ptr);
		}

		inline void set(const std::string &text) const {
			glfwSetClipboardString(app->engine.instance.window.ptr, text.c_str());
		}

		[[nodiscard]] static inline Clipboard of(const Element &element) {
			return Clipboard{element.getApp()};
		}

		[[nodiscard]] static inline Clipboard of(const WidgetStateBase *state) {
			return Clipboard{state->element->getApp()};
		}
	};
}// namespace squi::core