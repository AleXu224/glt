#pragma once

#include "child.hpp"
#include "key.hpp"
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
		std::string_view _name;

	public:
		friend Child;

		[[nodiscard]] const KeyBase &getKey() const {
			if (_key == nullptr || *_key == nullptr) {
				return *nullKey;
			}
			return **_key;
		}

		[[nodiscard]] size_t getTypeHash() const {
			return this->_typeHash;
		}

		[[nodiscard]] std::string_view getName() const {
			return this->_name;
		}

		[[nodiscard]] std::shared_ptr<Element> _createElement() const {
			assert(this->_createElementFunc != nullptr);
			return this->_createElementFunc();
		}

		[[nodiscard]] static bool canUpdate(const Child &oldWidget, const Child &newWidget) {
			if (oldWidget && newWidget) {
				return oldWidget->getTypeHash() == newWidget->getTypeHash() && oldWidget->getKey() == newWidget->getKey();
			}
			return false;
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

	// struct StatefulTestWidget : StatefulWidget {
	// 	Key key;
	// 	int b;

	// 	struct State : WidgetState<StatefulTestWidget> {
	// 		Child build(const Element &) override {
	// 			return {};
	// 		}
	// 	};
	// };

	// struct StatelessTestWidget : StatelessWidget {
	// 	Key key;
	// 	int c;

	// 	static Child build(const Element &) {
	// 		return {};
	// 	}
	// };

	// struct RenderObjectTestWidget : RenderObjectWidget {
	// 	Key key;
	// 	int d;

	// 	static std::shared_ptr<RenderObject> createRenderObject() {
	// 		return {};
	// 	}

	// 	void updateRenderObject(RenderObject *renderObject) const {
	// 		// Update render object properties here
	// 	}

	// 	static Args getArgs() {
	// 		return {};
	// 	}
	// };

	// inline void _test() {
	// 	WidgetPtr a = StatelessTestWidget{.c = 42};
	// 	WidgetPtr b = StatefulTestWidget{.b = 24};
	// 	WidgetPtr c = RenderObjectTestWidget{.d = 12};
	// }
}// namespace squi::core