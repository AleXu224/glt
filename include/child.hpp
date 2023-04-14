#ifndef GLT_CHILD_HPP
#define GLT_CHILD_HPP

#include <utility>

#include "memory"
#include "vector"

namespace squi {
	class Widget;

	class Child {
		std::shared_ptr<Widget> widget{};

	public:
		Child() = default;

		Child(std::shared_ptr<Widget> child) : widget(std::move(child)) {
			initializeWidget();
		}

		Child& operator=(std::shared_ptr<Widget> child) {
			this->widget = std::move(child);
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

		[[nodiscard]] std::shared_ptr<Widget> operator->() {
			return widget;
		}
	};

	class Children {
		std::vector<Child> children{};

	public:
		Children() = default;

		Children(std::vector<Child> children) : children(std::move(children)) {}

		Children(const std::vector<std::shared_ptr<Widget>>& children) {
			for (auto &child : children) {
				this->children.emplace_back(child);
			}
		}

		template<typename... Args>
		Children(Args... args) : children{args...} {}

		Children& operator=(std::vector<Child> children) {
			this->children = std::move(children);
			return *this;
		}

		[[nodiscard]] operator std::vector<std::shared_ptr<Widget>>() const;

		[[nodiscard]] bool hasChildren() const {
			return !children.empty();
		}
	};

}// namespace squi

#endif//GLT_CHILD_HPP
