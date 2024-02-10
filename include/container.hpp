#pragma once

#include "widget.hpp"
#include <memory>

namespace squi {
	struct Container {
		// Args
		Widget::Args widget{};
		Child child{};

		class Impl : public Widget {
		public:
			Impl(const Container &args) : Widget(args.widget, Widget::Flags::Default()) {
				addChild(args.child);
			}
		};

		operator Child() const {
			return {std::make_shared<Impl>(*this)};
		}
	};
}// namespace squi