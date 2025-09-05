#pragma once

#include "state.hpp"
#include <cassert>

namespace squi::core {
	struct Element : std::enable_shared_from_this<Element> {
		WidgetPtr widget;
		Element *parent = nullptr;
		bool dirty = true;
		bool mounted = false;

		Element(const WidgetPtr &widget) : widget(widget) {
			assert(widget != nullptr);
		};

		virtual ~Element() = default;

		virtual void mount(Element *parent) {
			this->dirty = true;
			this->parent = parent;
			this->mounted = true;
		}

		virtual void update(const WidgetPtr &newWidget) {
			assert(this->mounted);
			if (this->widget != newWidget) {
				this->widget = newWidget;
				this->dirty = true;
			}
		}

		virtual void rebuild() {
			assert(this->mounted);
			if (this->dirty) {
				this->dirty = false;
			}
		}

		virtual void unmount() {
			this->parent = nullptr;
			this->mounted = false;
		}
	};

	struct ComponentElement : Element {
		ElementPtr child;

		ComponentElement(const WidgetPtr &widget) : Element(widget) {};

		virtual void firstBuild();

		void mount(Element *parent) override {
			Element::mount(parent);
			this->firstBuild();
		}
	};

	struct StatelessElement : ComponentElement {
		StatelessElement(const WidgetPtr &widget) : ComponentElement(widget) {};

		void rebuild() override {
		}
	};

	struct StatefulElement : ComponentElement {
		std::shared_ptr<WidgetStateBase> state;

		StatefulElement(const WidgetPtr &widget);
	};
}// namespace squi::core