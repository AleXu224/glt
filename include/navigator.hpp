#pragma once

#include "widget.hpp"

namespace squi {
	struct Navigator {
		struct Controller {
			void push(Child child);
			void pop();

		private:
			friend Navigator;
			Child pushChild{};
			bool shouldPop{false};
		};

		// Args
		Widget::Args widget;
		Controller controller;
		Child child;

		class Impl : public Widget {
			// Data
			Controller controller;

		public:
			Impl(const Navigator &args);

			// void updateChildren() final;

			// void layoutChildren(vec2 &maxSize, vec2 &minSize) final;
			// void arrangeChildren(vec2 &pos) final;

			// void drawChildren() final;
		};

		operator Child() const {
			return std::make_shared<Impl>(*this);
		}
	};
}// namespace squi