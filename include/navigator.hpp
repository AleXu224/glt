#pragma once

#include "observer.hpp"
#include "widget.hpp"

namespace squi {

	struct Navigator {
		struct Controller {
			void push(const Child &child) const;
			void pop() const;

		private:
			friend Navigator;
			Observable<Child> pusher{};
			VoidObservable popper{};
		};
		// Args
		squi::Widget::Args widget{};
		Controller controller{};
		Child child{};
		struct Storage {
			// Data
			size_t lastId = 0;
			std::vector<size_t> ids{};
		};

		operator squi::Child() const;
	};
}// namespace squi