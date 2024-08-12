#pragma once

#include "widget.hpp"

namespace squi {
	struct SoftWrap {
		// Args
		squi::Widget::Args widget{};
		Child child{};

		class Impl : public squi::Widget {
			// Data

		public:
			vec2 layoutChildren(vec2 maxSize, vec2 minSize, ShouldShrink shouldShrink, bool final) override;

			Impl(const SoftWrap &args);
		};

		operator squi::Child() const {
			return std::make_shared<Impl>(*this);
		}
	};
}// namespace squi