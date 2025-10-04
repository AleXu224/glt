#pragma once

#include "renderObject.hpp"
#include "state.hpp"
#include <cassert>

namespace squi::core {
	struct Element : std::enable_shared_from_this<Element> {
		WidgetPtr widget;
		Element *parent = nullptr;
		Element *root = nullptr;
		size_t depth = 0;
		static inline uint64_t nextId = 1;
		const uint64_t id = nextId++;
		size_t index = 0;
		bool dirty = true;
		bool mounted = false;

		Element(const Element &) = delete;
		Element(Element &&) = delete;
		Element &operator=(const Element &) = delete;
		Element &operator=(Element &&) = delete;

		Element(const WidgetPtr &widget) : widget(widget) {
			assert(widget != nullptr);
		};

		virtual ~Element() = default;

		template<class T>
		T *getWidgetAs() const {
			return static_cast<T *>(widget.get());
		}

		virtual void mount(Element *parent, size_t index, size_t depth) {
			this->dirty = true;
			this->parent = parent;
			this->root = parent ? parent->root : this;
			this->mounted = true;
			this->index = index;
			this->depth = depth;
		}

		virtual void update(const WidgetPtr &newWidget) {
			assert(this->mounted);
			this->widget = newWidget;
		}

		virtual void rebuild();

		virtual void unmount() {
			this->parent = nullptr;
			this->root = nullptr;
			this->mounted = false;
		}

		App *getApp() const;

		void markNeedsRebuild();

		void markNeedsRelayout() const;
		void markNeedsReposition() const;
		void markNeedsRedraw() const;

		void addPostLayoutTask(const std::function<void()> &task) const;

		ElementPtr updateChild(ElementPtr child, const WidgetPtr &newWidget, size_t index, size_t depth);
		virtual void updateIndex(size_t index);
		void updateChildren(std::vector<ElementPtr> &oldChildren, const std::vector<WidgetPtr> &newWidgets);
	};

	struct ComponentElement : Element {
		ElementPtr child;

		ComponentElement(const WidgetPtr &widget) : Element(widget) {};

		virtual void firstBuild();
		virtual Child build() = 0;
		virtual void beforeRebuild() {};

		void mount(Element *parent, size_t index, size_t depth) override;
		void rebuild() override;
		void update(const WidgetPtr &newWidget) override;
		void unmount() override;

		void updateIndex(size_t index) override;
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
		void firstBuild() override;
		void beforeRebuild() override;
		void rebuild() override;
		void unmount() override;
	};

	struct RenderObjectElement : Element {
		std::shared_ptr<RenderObject> renderObject;

		RenderObjectElement(const RenderObjectWidgetPtr &widget);

		void mount(Element *parent, size_t index, size_t depth) override;
		void update(const WidgetPtr &newWidget) override;
		void unmount() override;

		void updateIndex(size_t index) override;

	private:
		void attachRenderObject();
		void detachRenderObject();
		static RenderObjectElement *getAncestorRenderObjectElement(Element *element);
	};

	struct SingleChildRenderObjectElement : RenderObjectElement {
		ElementPtr child;

		SingleChildRenderObjectElement(const RenderObjectWidgetPtr &widget) : RenderObjectElement(widget) {}

		virtual void firstBuild();
		virtual Child build() = 0;

		void mount(Element *parent, size_t index, size_t depth) override;
		void rebuild() override;
		void update(const WidgetPtr &newWidget) override;
		void unmount() override;
	};

	struct MultiChildRenderObjectElement : RenderObjectElement {
		std::vector<ElementPtr> children;

		MultiChildRenderObjectElement(const RenderObjectWidgetPtr &widget) : RenderObjectElement(widget) {}

		virtual void firstBuild();
		virtual std::vector<Child> build() = 0;
		std::vector<Child> buildAndPrune();

		void mount(Element *parent, size_t index, size_t depth) override;
		void rebuild() override;
		void update(const WidgetPtr &newWidget) override;
		void unmount() override;
	};
}// namespace squi::core