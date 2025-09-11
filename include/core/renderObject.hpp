#pragma once

#include "boxConstraints.hpp"
#include "memory"
#include "rect.hpp"
#include "vec2.hpp"
#include "vector"
#include <functional>
#include <optional>
#include <variant>


namespace squi::core {
	struct App;
	struct RenderObjectWidget;
	struct RenderObjectElement;

	enum class Size : uint8_t {
		// Will expand to fill all the available space
		Expand,
		// Will shrink to the minimum size
		Shrink,
		// Will will let the children expand, taking up the minimum size to contain the children afterwards
		Wrap,
	};

	struct Args {
		std::variant<float, Size> width = Size::Expand;
		std::variant<float, Size> height = Size::Expand;
		BoxConstraints sizeConstraints{};
		Margin margin{};
		Margin padding{};
	};

	struct RenderObject : std::enable_shared_from_this<RenderObject> {
		RenderObjectElement *element = nullptr;
		RenderObject *parent = nullptr;

		vec2 size{};
		vec2 pos{};

		std::variant<float, Size> width = Size::Expand;
		std::variant<float, Size> height = Size::Expand;
		BoxConstraints sizeConstraints{};
		Margin margin{};
		Margin padding{};

		RenderObject() = default;
		RenderObject(const RenderObject &) = default;
		RenderObject(RenderObject &&) = delete;
		RenderObject &operator=(const RenderObject &) = default;
		RenderObject &operator=(RenderObject &&) = delete;
		virtual ~RenderObject() = default;

		App *getApp() const;

		vec2 calculateSize(BoxConstraints constraints, bool final = false);
		virtual vec2 calculateContentSize(BoxConstraints constraints, bool final);

		void positionAt(const vec2 &newPos);
		virtual void positionContentAt(const vec2 &newPos) {}

		void draw();
		virtual void drawSelf() {}
		virtual void drawContent() {}

		[[nodiscard]] Rect getRect() const;
		[[nodiscard]] Rect getContentRect() const;
		[[nodiscard]] Rect getLayoutRect() const;

		virtual void iterateChildren(const std::function<void(RenderObject *)> &callback) {}

		virtual void addChild(std::shared_ptr<RenderObject> child, std::optional<size_t> index = std::nullopt) {
			assert(false);// Can't add children to this RenderObject
		}

		virtual void removeChild(std::shared_ptr<RenderObject> child) {
			assert(false);// Can't remove children from this RenderObject
		}

		void initRenderObject();
		void updateWidgetArgs(const Args &args);
		virtual void init() {}

		RenderObjectWidget *getWidget() const;

		template<class T>
		T *getWidgetAs() const {
			return static_cast<T *>(this->getWidget());
		}
	};

	struct SingleChildRenderObject : RenderObject {
		std::shared_ptr<RenderObject> child;

		SingleChildRenderObject() : RenderObject() {}

		vec2 calculateContentSize(BoxConstraints constraints, bool final) override;
		void positionContentAt(const vec2 &newPos) override;

		void drawContent() override;

		void iterateChildren(const std::function<void(RenderObject *)> &callback) override {
			if (child) {
				callback(child.get());
				child->iterateChildren(callback);
			}
		}

		void addChild(std::shared_ptr<RenderObject> child, std::optional<size_t> index = std::nullopt) override {
			if (child->parent) {
				child->parent->removeChild(child);
			}
			assert(this->child == nullptr);// Can only have one child
			this->child = child;
			child->parent = this;
			child->initRenderObject();
		}

		void removeChild(std::shared_ptr<RenderObject> child) override {
			assert(this->child == child);
			this->child = nullptr;
			child->parent = nullptr;
		}
	};

	struct MultiChildRenderObject : RenderObject {
		std::vector<std::shared_ptr<RenderObject>> children;

		MultiChildRenderObject() : RenderObject() {}

		vec2 calculateContentSize(BoxConstraints constraints, bool final) override;
		void positionContentAt(const vec2 &newPos) override;

		void drawContent() override;

		void iterateChildren(const std::function<void(RenderObject *)> &callback) override {
			for (const auto &child: children) {
				callback(child.get());
				child->iterateChildren(callback);
			}
		}

		void addChild(std::shared_ptr<RenderObject> child, std::optional<size_t> index = std::nullopt) override {
			if (child->parent) {
				child->parent->removeChild(child);
			}
			if (index) {
				children.insert(children.begin() + static_cast<std::vector<std::shared_ptr<RenderObject>>::difference_type>(*index), child);
			} else {
				children.push_back(child);
			}
			child->parent = this;
			child->initRenderObject();
		}

		void removeChild(std::shared_ptr<RenderObject> child) override {
			auto it = std::find(children.begin(), children.end(), child);
			if (it != children.end()) {
				children.erase(it);
				child->parent = nullptr;
			}
		}
	};
}// namespace squi::core
