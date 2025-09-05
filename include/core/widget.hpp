#pragma once

#include "child.hpp"
#include "element.hpp"
#include "state.hpp"
#include <functional>

namespace squi::core {
	struct StatelessWidget;
	struct StatefulWidget;

	template<class T>
	concept StateLike = requires(T a) {
		{ a.build(std::declval<const Element &>()) } -> std::same_as<WidgetPtr>;
	};

	template<class T>
	concept StatefulWidgetLike = requires(T a) {
		typename std::remove_cvref_t<T>::State;
		requires StateLike<typename std::remove_cvref_t<T>::State>;
	};

	template<class T>
	concept StatelessWidgetLike = requires(T a) {
		{ a.build(std::declval<const Element &>()) } -> std::same_as<WidgetPtr>;
	};

	struct Widget {
	private:
		std::function<std::shared_ptr<Element>()> _createElement = nullptr;
		std::function<std::shared_ptr<WidgetStateBase>()> _createState = nullptr;
		std::function<WidgetPtr(const Element &)> _build = nullptr;

	public:
		std::shared_ptr<Element> createElement() const {
			assert(this->_createElement != nullptr);
			return this->_createElement();
		}

		std::shared_ptr<WidgetStateBase> createState() const {
			assert(this->_createState != nullptr);
			return this->_createState();
		}

		WidgetPtr build(const Element &element) const {
			assert(this->_build != nullptr);
			return this->_build(element);
		}

		operator WidgetPtr(this auto &&self) {
			using Self = std::remove_cvref_t<decltype(self)>;

			if constexpr (std::is_base_of_v<StatefulWidget, Self>) {
				static_assert(StatefulWidgetLike<Self>, "StatefulWidget must be stateful");
				auto widget = std::make_shared<Self>(std::forward<decltype(self)>(self));

				widget->_createElement = [widget = std::weak_ptr<Self>(widget)]() -> std::shared_ptr<Element> {
					auto ptr = widget.lock();
					assert(ptr != nullptr);
					return std::make_shared<StatefulElement>(ptr);
				};

				widget->_createState = []() -> std::shared_ptr<WidgetStateBase> {
					return std::make_shared<typename Self::State>();
				};

				widget->_build = [widget = std::weak_ptr<Self>(widget)](const Element &element) -> Child {
					auto ptr = widget.lock();
					assert(ptr != nullptr);
					return ptr->build(element);
				};

				return widget;
			} else if constexpr (std::is_base_of_v<StatelessWidget, Self>) {
				static_assert(StatelessWidgetLike<Self>, "StatelessWidget must be stateless");
				auto widget = std::make_shared<Self>(std::forward<decltype(self)>(self));

				widget->_createElement = [widget = std::weak_ptr<Self>(widget)]() -> std::shared_ptr<Element> {
					auto ptr = widget.lock();
					assert(ptr != nullptr);
					return std::make_shared<StatelessElement>(ptr);
				};

				return widget;
			} else {
				static_assert(false, "Invalid widget type");
			}
		}
	};

	struct StatelessWidget : Widget {};
	struct StatefulWidget : Widget {};

	struct Stateful : StatefulWidget {
		int b;

		struct State : WidgetState<Stateful> {
			Child build(const Element &element) override {
				return {};
			}
		};
	};

	struct Stateless : StatelessWidget {
		int c;

		Child build(const Element &element) const {
			return {};
		}
	};


	void test() {
		WidgetPtr a = Stateless{.c = 42};
		WidgetPtr b = Stateful{.b = 24};
	}
}// namespace squi::core