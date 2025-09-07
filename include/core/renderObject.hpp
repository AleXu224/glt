#pragma once

#include "boxConstraints.hpp"
#include "child.hpp"
#include "memory"
#include "vec2.hpp"
#include "vector"
#include <variant>


namespace squi::core {
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

	struct RenderObject {
		RenderObject *parent = nullptr;

		vec2 size{};
		vec2 pos{};

		std::variant<float, Size> width;
		std::variant<float, Size> height;
		BoxConstraints sizeConstraints;
		Margin margin;
		Margin padding;

		RenderObject(const Args &args = Args{})
			: width(args.width),
			  height(args.height),
			  sizeConstraints(args.sizeConstraints),
			  margin(args.margin),
			  padding(args.padding) {}
		virtual ~RenderObject() = default;

		vec2 calculateSize(BoxConstraints constraints, bool final = false);
		virtual vec2 calculateContentSize(BoxConstraints constraints, bool final);

		void positionAt(const vec2 &newPos);
		virtual void positionContentAt(const vec2 &newPos) {}

		void draw();
		virtual void drawSelf() {}
		virtual void drawContent() {}

		virtual void addChild(std::shared_ptr<RenderObject> child) {
			assert(false);// Can't add children to this RenderObject
		}

		virtual void removeChild(std::shared_ptr<RenderObject> child) {
			assert(false);// Can't remove children from this RenderObject
		}
	};

	struct SingleChildRenderObject : RenderObject {
		std::shared_ptr<RenderObject> child;

		SingleChildRenderObject(const Args &args = Args{}) : RenderObject(args) {}

		vec2 calculateContentSize(BoxConstraints constraints, bool final) override;
		void positionContentAt(const vec2 &newPos) override;

		void addChild(std::shared_ptr<RenderObject> child) override {
			if (child->parent) {
				child->parent->removeChild(child);
			}
			assert(this->child == nullptr);// Can only have one child
			this->child = child;
			child->parent = this;
		}

		void removeChild(std::shared_ptr<RenderObject> child) override {
			assert(this->child == child);
			this->child = nullptr;
			child->parent = nullptr;
		}
	};

	struct MultiChildRenderObject : RenderObject {
		std::vector<std::shared_ptr<RenderObject>> children;

		MultiChildRenderObject(const Args &args = Args{}) : RenderObject(args) {}

		vec2 calculateContentSize(BoxConstraints constraints, bool final) override;
		void positionContentAt(const vec2 &newPos) override;

		void addChild(std::shared_ptr<RenderObject> child) override {
			if (child->parent) {
				child->parent->removeChild(child);
			}
			children.push_back(child);
			child->parent = this;
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
