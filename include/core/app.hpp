#pragma once

#include "widget.hpp"

namespace squi::core {
	struct RootRenderObject : RenderObject {
		void performLayout() override {
			// Layout logic for the root render object
		}

		void paint() override {
			// Painting logic for the root render object
		}
	};

	struct RootElement : Element {
		RootElement(const WidgetPtr &widget) : Element(widget) {}
	};

	struct App : RootRenderObject {
		Child child;
		void run();
	};
}// namespace squi::core