#ifndef GLT_CHILD_HPP
#define GLT_CHILD_HPP

#include <utility>
#include <xmemory>

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

		[[nodiscard]] Widget *getAddress() const {
			return widget.get();
		}
	};

	class Children {
		std::vector<Child> children{};

	public:
		Children() = default;

		Children(std::vector<Child> children) : children(std::move(children)) {
			children.erase(std::remove_if(children.begin(), children.end(), [](const Child &child) {
				return !child.hasChild();
			}), children.end());
		}

		Children(const std::vector<std::shared_ptr<Widget>>& children) {
			for (auto &child : children) {
				if (child) this->children.emplace_back(child);
			}
		}

		template<typename... Args>
		Children(Args... args) : children{args...} {
			children.erase(std::remove_if(children.begin(), children.end(), [](const Child &child) {
				return !child.hasChild();
			}), children.end());
		}

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
