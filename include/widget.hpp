#ifndef SQUI_WIDGEt_HPP
#define SQUI_WIDGEt_HPP

#include "functional"
#include "margin.hpp"
#include "memory"
#include "sizeBehavior.hpp"
#include "vec2.hpp"
#include "vector"

namespace squi {
	class Widget {
		struct Args {
			vec2 size;
			Margin margin;
			Margin padding;
			SizeBehavior sizeBehavior;
			std::function<void(Widget *)> onInit{};
			std::function<void(Widget *)> beforeUpdate{};
			std::function<void(Widget *)> onUpdate{};
			std::function<void(Widget *)> afterUpdate{};
		};

		Args args;
		bool isContainer = true;
		vec2 pos{};
		Widget *parent = nullptr;
		std::vector<std::shared_ptr<Widget>> children;

	public:
		// Disable copy
		Widget(const Widget &) = delete;
		Widget &operator=(const Widget &) = delete;
		// Disable move
		Widget(Widget &&) = delete;
		Widget &operator=(Widget &&) = delete;

		Widget(const Args &args, bool isContainer = true);

		// Getters
		[[nodiscard]] const vec2 &getSize() const;
		[[nodiscard]] const vec2 &getPos() const;
		[[nodiscard]] const Margin &getMargin() const;
		[[nodiscard]] const Margin &getPadding() const;
		[[nodiscard]] const SizeBehavior &getSizeBehavior() const;
		[[nodiscard]] Widget &getParent() const;
		[[nodiscard]] const std::vector<std::shared_ptr<Widget>> &getChildren() const;

		// Setters
		void setSize(const vec2 &newSize);
		void setPos(const vec2 &newPos);
		void setMargin(const Margin &newMargin);
		void setPadding(const Margin &newPadding);
		void setSizeBehavior(const SizeBehavior &newSizeBehavior);
		void setParent(Widget *newParent);
		void setChildren(const std::vector<std::shared_ptr<Widget>> &newChildren);

		// Methods
		void addChild(std::shared_ptr<Widget> child);
		void update();
		void draw();

		// Virtual methods
		virtual void onUpdate();
		virtual void afterChildrenUpdate();
		virtual void onDraw();
	};
}// namespace squi

#endif