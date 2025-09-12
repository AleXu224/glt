#pragma once

#include "child.hpp"
#include "concepts.hpp"
#include "element.hpp"
#include "key.hpp"
#include "renderObject.hpp"
#include "state.hpp"
#include <functional>


namespace squi::core {
	struct StatelessWidget;
	struct StatefulWidget;
	struct RenderObjectWidget;

	struct Widget {
	private:
		std::function<std::shared_ptr<Element>()> _createElementFunc = nullptr;
		Key *_key = nullptr;
		size_t _typeHash = 0;

	public:
		[[nodiscard]] const KeyBase &getKey() const {
			if (_key == nullptr || *_key == nullptr) {
				return *nullKey;
			}
			return **_key;
		}

		[[nodiscard]] size_t getTypeHash() const {
			return this->_typeHash;
		}

		[[nodiscard]] std::shared_ptr<Element> _createElement() const {
			assert(this->_createElementFunc != nullptr);
			return this->_createElementFunc();
		}

		[[nodiscard]] static bool canUpdate(const WidgetPtr &oldWidget, const WidgetPtr &newWidget) {
			if (oldWidget && newWidget) {
				return oldWidget->getTypeHash() == newWidget->getTypeHash() && oldWidget->getKey() == newWidget->getKey();
			}
			return false;
		}

		operator WidgetPtr(this auto &&self) {
			using Self = std::remove_cvref_t<decltype(self)>;

			self._typeHash = typeid(self).hash_code();
			static_assert(HasKey<Self>, "Widget requires a key");
			auto widget = std::make_shared<Self>(std::forward<decltype(self)>(self));
			widget->_key = &widget->key;

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

				return widget;
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

				return widget;
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

				return widget;
			} else {
				static_assert(false, "Invalid widget type");
			}
		}
	};

	struct StatelessWidget : Widget {
		std::function<WidgetPtr(const Element &)> _buildFunc = nullptr;

		[[nodiscard]] WidgetPtr _build(const Element &element) const {
			assert(this->_buildFunc != nullptr);
			return this->_buildFunc(element);
		}
	};

	struct StatefulWidget : Widget {
		std::function<std::shared_ptr<WidgetStateBase>()> _createStateFunc = nullptr;
		std::function<WidgetPtr(const Element &)> _buildFunc = nullptr;

		[[nodiscard]] std::shared_ptr<WidgetStateBase> _createState() const {
			assert(this->_createStateFunc != nullptr);
			return this->_createStateFunc();
		}

		[[nodiscard]] WidgetPtr _build(const Element &element) const {
			assert(this->_buildFunc != nullptr);
			return this->_buildFunc(element);
		}
	};

	struct RenderObjectWidget : Widget {
		std::function<std::shared_ptr<RenderObject>()> _createRenderObjectFunc = nullptr;
		std::function<void(RenderObject *)> _updateRenderObjectFunc = nullptr;
		std::function<Args()> _getWidgetArgsFunc = nullptr;

		[[nodiscard]] std::shared_ptr<RenderObject> _createRenderObject() const {
			assert(this->_createRenderObjectFunc != nullptr);
			auto ret = this->_createRenderObjectFunc();
			assert(ret != nullptr);
			return ret;
		}

		void _updateRenderObject(RenderObject *renderObject) const {
			assert(this->_updateRenderObjectFunc != nullptr);
			this->_updateRenderObjectFunc(renderObject);
		}

		[[nodiscard]] Args _getWidgetArgs() const {
			assert(this->_getWidgetArgsFunc != nullptr);
			return this->_getWidgetArgsFunc();
		}
	};

	struct StatefulTestWidget : StatefulWidget {
		Key key;
		int b;

		struct State : WidgetState<StatefulTestWidget> {
			Child build(const Element &) override {
				return {};
			}
		};
	};

	struct StatelessTestWidget : StatelessWidget {
		Key key;
		int c;

		static Child build(const Element &) {
			return {};
		}
	};

	struct RenderObjectTestWidget : RenderObjectWidget {
		Key key;
		int d;

		static std::shared_ptr<RenderObject> createRenderObject() {
			return {};
		}

		void updateRenderObject(RenderObject *renderObject) const {
			// Update render object properties here
		}

		static Args getArgs() {
			return {};
		}
	};

	inline void _test() {
		WidgetPtr a = StatelessTestWidget{.c = 42};
		WidgetPtr b = StatefulTestWidget{.b = 24};
		WidgetPtr c = RenderObjectTestWidget{.d = 12};
	}
}// namespace squi::core