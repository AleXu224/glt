#ifndef GLT_CHILD_HPP
#define GLT_CHILD_HPP

#include "memory"

namespace squi {
	class Widget;

	class Child {
		std::shared_ptr<Widget> widget{};

	public:
		Child() = default;

		template<class T>
		Child(T *child) requires(std::is_base_of_v<Widget, T>) : widget(std::shared_ptr<Widget>(child)) {
			initializeWidget();
		}

		template<class T>
		Child& operator=(T *child) requires(std::is_base_of_v<Widget, T>) {
			this->widget.reset(child);
			initializeWidget();
			return *this;
		}

		void initializeWidget();

		[[nodiscard]] operator std::shared_ptr<Widget>() const;

		[[nodiscard]] std::shared_ptr<Widget> operator->() const {
			return widget;
		}

		[[nodiscard]] bool hasChild() const {
			return widget != nullptr;
		}
	};

}// namespace squi

#endif//GLT_CHILD_HPP
