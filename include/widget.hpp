#ifndef SQUI_WIDGEt_HPP
#define SQUI_WIDGEt_HPP

#include "functional"
#include "margin.hpp"
#include "memory"
#include "rect.hpp"
#include "sizeBehavior.hpp"
#include "vec2.hpp"
#include "vector"
#include "child.hpp"

namespace squi {
	class Widget {
	public:
		struct Args {
			/**
			 * The size of the Widget.
			 * Will not be affected by the margin or padding.
			 * Can be overriden by the size behavior.
			 */
			vec2 size;
			/**
			 * The space around the widget. Does not affect the size.
			 */
			Margin margin;
			/**
			 * The space inside the widget. Does not affect the size.
			 */
			Margin padding;
			/**
			 * The size behavior of the widget.
			 * This will override the size of the widget.
			 */
			SizeBehavior sizeBehavior;
			std::function<void(Widget &)> onInit{};
			std::function<void(Widget &)> beforeUpdate{};
			std::function<void(Widget &)> onUpdate{};
			std::function<void(Widget &)> afterUpdate{};
		};

		struct Options {
			/**
			 * When true this Widget will relay all the
			 */
			bool isContainer = true;
			/**
			 * Set to false to manually update the children
			 */
			bool shouldUpdateChildren = true;
			/**
			 * Set to false to manually draw the children
			 */
			bool shouldDrawChildren = true;
			/**
			 * Set to false to manually handle the size behavior
			 */
			bool shouldHandleSizeBehavior = true;
			/**
			 * Whether hit testing should be performed on this widget
			 */
			bool isInteractive = true;
		};

	private:
		Args data;
		bool isContainer;
		bool shouldUpdateChildren;
		bool shouldDrawChildren;
		bool shouldHandleSizeBehavior;
		bool isInteractive;
		bool isInitialized = false;
		vec2 pos{};
		vec2 sizeHint{-1, -1};
		Widget *parent = nullptr;
		std::vector<std::shared_ptr<Widget>> children{};

		[[nodiscard]] const Args &getData() const;
		[[nodiscard]] Args &getData();

	public:
		// Disable copy
		Widget(const Widget &) = delete;
		Widget &operator=(const Widget &) = delete;
		// Disable move
		Widget(Widget &&) = delete;
		Widget &operator=(Widget &&) = delete;

		explicit Widget(Args args, const Options &options);
		virtual ~Widget() = default;

		// Getters
		[[nodiscard]] const vec2 &getSize() const;
		[[nodiscard]] const vec2 &getPos() const;
		[[nodiscard]] const vec2 &getSizeHint() const;
		[[nodiscard]] const Margin &getMargin() const;
		[[nodiscard]] const Margin &getPadding() const;
		[[nodiscard]] const SizeBehavior &getSizeBehavior() const;
		[[nodiscard]] Widget &getParent() const;
		[[nodiscard]] const std::vector<std::shared_ptr<Widget>> &getChildren() const;
		[[nodiscard]] virtual Rect getRect() const;
		[[nodiscard]] virtual Rect getContentRect() const;
		[[nodiscard]] virtual Rect getLayoutRect() const;
		[[nodiscard]] virtual std::vector<Rect> getHitcheckRect() const;

		// Setters
		void setSize(const vec2 &newSize);
		void setPos(const vec2 &newPos);
		void setSizeHint(const vec2 &newSizeHint);
		void setMargin(const Margin &newMargin);
		void setPadding(const Margin &newPadding);
		void setSizeBehavior(const SizeBehavior &newSizeBehavior);
		void setParent(Widget *newParent);
		void setChildren(const std::vector<std::shared_ptr<Widget>> &newChildren);

		// Methods
		void addChild(const Child &child);
		void update();
		void draw();
		void initialize();

		// Virtual methods
		virtual void init(){};
		virtual void onUpdate(){};
		virtual void afterChildrenUpdate(){};
		virtual void afterUpdate(){};
		virtual void onDraw(){};
	};
}// namespace squi

#endif