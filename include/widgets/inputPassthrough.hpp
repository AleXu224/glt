#pragma once

#include "core/core.hpp"
#include "misc/gestureEnums.hpp"

namespace squi {
	struct InputPassthrough : RenderObjectWidget {
		Key key;
		Args widget{};
		// The InputLevel to grand to the parent Gesture of this widget. If an override already exists then the max of the two is used
		InputLevel passthrough = InputLevel::none;
		// If set, replaces the InputLevel granted to the parent
		std::optional<InputLevel> override{};
		// The maximum distance the cursor can move before a click is no longer registered
		std::optional<float> dragThreshold{};
		Child child;

		struct Element : SingleChildRenderObjectElement {
			using SingleChildRenderObjectElement::SingleChildRenderObjectElement;

			Child build() override {
				if (auto flags = std::static_pointer_cast<InputPassthrough>(widget)) {
					return flags->child;
				}
				return {};
			}
		};

		struct InputPassthroughRenderObject : SingleChildRenderObject {};

		static std::shared_ptr<RenderObject> createRenderObject() {
			return std::make_shared<InputPassthroughRenderObject>();
		}

		void updateRenderObject(RenderObject *renderObject) const {}

		[[nodiscard]] Args getArgs() const {
			auto ret = widget;
			ret.width = ret.width.value_or(Size::Wrap);
			ret.height = ret.height.value_or(Size::Wrap);
			return ret;
		}
	};
}// namespace squi
