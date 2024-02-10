#pragma once

#include "widget.hpp"
#include <memory>

namespace squi {
	struct Stack {
		// Args
		Widget::Args widget{};
		Children children{};

		class Impl : public Widget {
			// Data

		public:
			Impl(const Stack &args);

			void updateChildren() final;

			vec2 layoutChildren(vec2 maxSize, vec2 minSize, ShouldShrink shouldShrink) final;

			std::vector<Rect> getHitcheckRect() const final;
		};

		operator Child() const {
			return std::make_shared<Impl>(*this);
		}
	};
}// namespace squi