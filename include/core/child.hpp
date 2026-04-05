#pragma once

#include "concepts.hpp"
#include "element.hpp"
#include "renderObject.hpp"


#include "memory"


namespace squi::core {
	struct Child {
		std::shared_ptr<Widget> widget{};

		Child() = default;
		Child(const Child &) = default;
		Child(Child &&) = default;
		Child &operator=(const Child &) = default;
		Child &operator=(Child &&) = default;

		Child(nullptr_t) : widget(nullptr) {};
		Child(const std::shared_ptr<Widget> &widget) : widget(widget) {}

		operator bool() const {
			return widget != nullptr;
		}

		std::shared_ptr<Widget> operator->() const {
			return widget;
		}

		operator std::shared_ptr<Widget>() const {
			return widget;
		}

		Widget *get() const {
			return widget.get();
		}

		template<class T>
			requires std::derived_from<T, Widget>
		Child(const std::shared_ptr<T> &widget) : widget(widget) {}

		template<class T>
			requires std::derived_from<T, Widget>
		Child(T &&self) {
			using Self = std::remove_cvref_t<T>;

			self._typeHash = typeid(self).hash_code();
			static_assert(HasKey<Self>, "Widget requires a key");
			auto widget = std::make_shared<Self>(std::forward<decltype(self)>(self));
			widget->_key = &widget->key;

			widget->_name = typeid(self).name();

			if constexpr (std::is_base_of_v<StatefulWidget, Self>) {
				static_assert(StatefulWidgetLike<Self>, "StatefulWidget must be stateful");

				widget->_createElementFunc = [widget = std::weak_ptr<Self>(widget)]() -> std::shared_ptr<Element> {
					auto ptr = widget.lock();
					assert(ptr != nullptr);
					if constexpr (HasElement<Self>) {
						return std::make_shared<typename Self::Element>(ptr);
					}
					return std::make_shared<StatefulElement>(ptr);
				};

				widget->_createStateFunc = []() -> std::shared_ptr<WidgetStateBase> {
					return std::make_shared<typename Self::State>();
				};

				widget->_buildFunc = [widget = std::weak_ptr<Self>(widget)](const Element &element) -> WidgetPtr {
					auto ptr = widget.lock();
					assert(ptr != nullptr);
					return ptr->_build(element);
				};

				this->widget = widget;
				return;
			} else if constexpr (std::is_base_of_v<StatelessWidget, Self>) {
				static_assert(StatelessWidgetLike<Self>, "StatelessWidget must be stateless");

				widget->_createElementFunc = [widget = std::weak_ptr<Self>(widget)]() -> std::shared_ptr<Element> {
					auto ptr = widget.lock();
					assert(ptr != nullptr);
					if constexpr (HasElement<Self>) {
						return std::make_shared<typename Self::Element>(ptr);
					}
					return std::make_shared<StatelessElement>(ptr);
				};

				widget->_buildFunc = [widget = std::weak_ptr<Self>(widget)](const Element &element) -> WidgetPtr {
					auto ptr = widget.lock();
					assert(ptr != nullptr);
					return ptr->build(element);
				};

				this->widget = widget;
				return;
			} else if constexpr (std::is_base_of_v<RenderObjectWidget, Self>) {
				static_assert(RenderObjectWidgetLike<Self>, "RenderObjectWidget is missing required methods");

				if constexpr (HasWidgetArgsGetter<Self>) {
					widget->_getWidgetArgsFunc = [widget = std::weak_ptr<Self>(widget)]() -> Args {
						auto ptr = widget.lock();
						assert(ptr != nullptr);
						return ptr->getArgs();
					};
				} else if constexpr (HasWidgetArgs<Self>) {
					widget->_getWidgetArgsFunc = [widget = std::weak_ptr<Self>(widget)]() -> Args {
						auto ptr = widget.lock();
						assert(ptr != nullptr);
						return ptr->widget;
					};
				} else {
					widget->_getWidgetArgsFunc = []() -> Args {
						return {};
					};
				}

				widget->_createElementFunc = [widget = std::weak_ptr<Self>(widget)]() -> std::shared_ptr<Element> {
					auto ptr = widget.lock();
					assert(ptr != nullptr);
					if constexpr (HasElement<Self>) {
						return std::make_shared<typename Self::Element>(ptr);
					}
					return std::make_shared<RenderObjectElement>(ptr);
				};

				widget->_createRenderObjectFunc = [widget = std::weak_ptr<Self>(widget)]() -> std::shared_ptr<RenderObject> {
					auto ptr = widget.lock();
					assert(ptr != nullptr);
					return ptr->createRenderObject();
				};

				widget->_updateRenderObjectFunc = [widget = std::weak_ptr<Self>(widget)](RenderObject *renderObject) -> void {
					auto ptr = widget.lock();
					assert(ptr != nullptr);
					ptr->updateRenderObject(renderObject);
				};

				this->widget = widget;
				return;
			} else {
				static_assert(false, "Invalid widget type");
			}
		}
	};

	inline bool operator==(const std::shared_ptr<Widget> &lhs, const Child &rhs) {
		return lhs == rhs.widget;
	}

	inline bool operator==(const Child &lhs, const std::shared_ptr<Widget> &rhs) {
		return lhs.widget == rhs;
	}
}// namespace squi::core