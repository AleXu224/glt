#pragma once

#include "column.hpp"
#include "observer.hpp"
#include "widget.hpp"
#include <memory>


namespace squi {
	struct Scrollable {
		struct Controller {
			float viewHeight{0.0f};
			float contentHeight{0.0f};
			float scroll{0.0f};
			Observable<float> onScrollChange{};
		};
		// Args
		Widget::Args widget{};
		Column::Alignment alignment{Column::Alignment::left};
		float spacing{0.0f};
		/**
         * @brief onScroll(float scroll, float contentHeight, float viewHeight)
         */
		std::function<void(float, float, float)> onScroll{};
		std::shared_ptr<Controller> controller{std::make_shared<Controller>()};
		Children children{};

		class Impl : public Widget {
		public:
			float scroll = 0;
			float spacing;
			float horizontalOffsetFactor;
			float contentHeight = 0.f;
			bool scrolled = false;
			std::shared_ptr<Controller> controller;
			std::function<void(float, float, float)> onScroll{};

			Impl(const Scrollable &args);

			void onUpdate() final;
			void afterUpdate() final;
			vec2 layoutChildren(vec2 maxSize, vec2 minSize, ShouldShrink shouldShrink) final;
			void postLayout(vec2 &size) final;
			void arrangeChildren(vec2 &pos) final;

			void drawChildren() final;
		};

		operator Child() const;
	};
}// namespace squi