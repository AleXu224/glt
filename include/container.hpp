#ifndef SQUI_CONTAINER_HPP
#define SQUI_CONTAINER_HPP

#include "widget.hpp"

namespace squi {
	struct Container {
		// Args
		Widget::Args widget;
		Child child;

		class Impl : public Widget {
		public:
			Impl(const Container &args) : Widget(args.widget, Widget::Options::Default()){
                addChild(args.child);
            }
		};

		operator Child() const {
			return {std::make_shared<Impl>(*this)};
		}
	};
}// namespace squi

#endif