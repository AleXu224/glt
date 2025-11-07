#pragma once

#include "core/animated.hpp"
#include "core/core.hpp"

namespace squi {
	struct LayoutInspectorItem : StatefulWidget {
		// Args
		Key key;
		std::weak_ptr<RenderObject> renderObjectPtr;
		std::function<void(const std::pair<bool, std::weak_ptr<RenderObject>> &)> onHoverRenderObject;
		std::function<void(const std::weak_ptr<RenderObject> &)> onSelectRenderObject;

		struct State final : WidgetState<LayoutInspectorItem> {
			bool open = false;
			Animated<float> iconRotate{.from = 0.f};
			Animated<float> iconOffset{.from = 0.f};

			void initState() override {
				iconRotate.mount(this);
				iconOffset.mount(this);
			}

			[[nodiscard]] static std::string getElementName(const RenderObjectPtr &renderObject);

			Child build(const Element &) override;
		};
	};

	struct LayoutInspector : StatefulWidget {
		// Args
		Key key;
		Child child;

		struct State : WidgetState<LayoutInspector> {
			std::weak_ptr<RenderObject> contentRenderObject;
			bool visible = false;
			std::weak_ptr<RenderObject> hoveredRenderObject;
			std::weak_ptr<RenderObject> selectedRenderObject;
			std::function<void(const std::pair<bool, std::weak_ptr<RenderObject>> &)> onHoverRenderObject;
			std::function<void(const std::weak_ptr<RenderObject> &)> onSelectRenderObject;

			void initState() override;

			Child build(const Element &) override;
		};
	};
}// namespace squi