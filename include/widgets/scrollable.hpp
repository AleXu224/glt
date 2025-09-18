#pragma once

#include "core/core.hpp"
#include "observer.hpp"

namespace squi {
	struct Scrollable : RenderObjectWidget {
		struct Controller {
			float viewMainAxis{0.0f};
			float contentMainAxis{0.0f};
			float scroll{0.0f};
			Observable<float> onScrollChange{};
		};

		enum class Alignment : uint8_t {
			begin,
			center,
			end
		};

		Key key;
		Args widget;
		Alignment alignment = Alignment::begin;
		Axis direction = Axis::Vertical;
		float spacing = 0.0f;
		/**
         * @brief onScroll(float scroll, float contentHeight, float viewHeight)
         */
		std::function<void(float, float, float)> onScroll{};
		std::shared_ptr<Controller> controller{std::make_shared<Controller>()};
		Children children{};

		struct Element : MultiChildRenderObjectElement {
			using MultiChildRenderObjectElement::MultiChildRenderObjectElement;

			std::vector<Child> build() override {
				if (auto scrollableWidget = std::static_pointer_cast<Scrollable>(widget)) {
					return scrollableWidget->children;
				}
				return {};
			};
		};

		struct ScrollableRenderObject : MultiChildRenderObject {
			float scroll = 0;
			float contentMainAxis = 0.f;
			float spacing = 0;
			Alignment alignment = Alignment::begin;
			Axis direction = Axis::Vertical;
			std::shared_ptr<Controller> controller;
			std::function<void(float, float, float)> onScroll{};

			void init() override;
		};

		static std::shared_ptr<RenderObject> createRenderObject() {
			return std::make_shared<ScrollableRenderObject>();
		}

		void updateRenderObject(RenderObject *renderObject) const;

		static Args getArgs() {
			return {};
		}
	};
}// namespace squi