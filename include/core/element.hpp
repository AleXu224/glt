#pragma once

#include "renderObject.hpp"
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
			this->widget = newWidget;
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

		void markNeedsRebuild() {
			this->dirty = true;
			// FIXME: add to a global dirty list for rebuild scheduling
		}

		ElementPtr updateChild(ElementPtr child, WidgetPtr newWidget);
		void updateChildren(std::vector<ElementPtr> &oldChildren, const std::vector<WidgetPtr> &newWidgets);
	};

	struct ComponentElement : Element {
		ElementPtr child;

		ComponentElement(const WidgetPtr &widget) : Element(widget) {};

		virtual void firstBuild();
		virtual Child build() = 0;

		void mount(Element *parent) override;
		void rebuild() override;
		void update(const WidgetPtr &newWidget) override;
		void unmount() override;
	};

	struct StatelessElement : ComponentElement {
		StatelessElement(const StatelessWidgetPtr &widget);

		void update(const WidgetPtr &newWidget) override {
			ComponentElement::update(newWidget);
		}

		Child build() override;
	};

	struct StatefulElement : ComponentElement {
		std::shared_ptr<WidgetStateBase> state;
		StatefulElement(const StatefulWidgetPtr &widget);

		Child build() override;

		void update(const WidgetPtr &newWidget) override;
		void rebuild() override;
		void unmount() override;
	};

	struct RenderObjectElement : Element {
		std::shared_ptr<RenderObject> renderObject;

		RenderObjectElement(const RenderObjectWidgetPtr &widget);

		void mount(Element *parent) override;
		void update(const WidgetPtr &newWidget) override;
		void unmount() override;

	private:
		void attachRenderObject();
		void detachRenderObject();
	};

	struct SingleChildRenderObjectElement : RenderObjectElement {
		ElementPtr child;

		SingleChildRenderObjectElement(const RenderObjectWidgetPtr &widget) : RenderObjectElement(widget) {}

		virtual void firstBuild();
		virtual Child build() = 0;

		void mount(Element *parent) override;
		void rebuild() override;
		void update(const WidgetPtr &newWidget) override;
		void unmount() override;
	};
}// namespace squi::core