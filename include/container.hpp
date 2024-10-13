#pragma once

#include "widget.hpp"
#include <memory>

namespace squi {
	struct Container {
		// Args
		Widget::Args widget{};
		// If the content inside should be bound by the size of the container
		bool bounded = true;
		Child child{};

		class Impl : public Widget {
			bool bounded;
		public:
			Impl(const Container &args);

			vec2 layoutChildren(vec2 maxSize, vec2 minSize, ShouldShrink shouldShrink, bool final) override;
		};

		operator Child() const {
			return {std::make_shared<Impl>(*this)};
		}
	};
}// namespace squi