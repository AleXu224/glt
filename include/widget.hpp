#ifndef SQUI_WIDGEt_HPP
#define SQUI_WIDGEt_HPP

#include "child.hpp"
#include "functional"
#include "margin.hpp"
#include "memory"
#include "rect.hpp"
#include "sizeBehavior.hpp"
#include "vec2.hpp"
#include "vector"

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
		bool isInitialized = false;
		bool isContainer;
		bool shouldUpdateChildren;
		bool shouldDrawChildren;
		bool shouldHandleSizeBehavior;
		bool isInteractive;
		std::function<void(Widget &)> onInitArg;
		std::function<void(Widget &)> beforeUpdateArg;
		std::function<void(Widget &)> onUpdateArg;
		std::function<void(Widget &)> afterUpdateArg;
		struct Data {
			vec2 size;
			Margin margin;
			Margin padding;
			SizeBehavior sizeBehavior;
			vec2 pos{};
			vec2 sizeHint{-1, -1};
			Widget *parent = nullptr;
		};
		Data m_data;
		std::vector<std::shared_ptr<Widget>> children{};

	public:
		// Disable copy
		Widget(const Widget &) = delete;
		Widget &operator=(const Widget &) = delete;
		// Disable move
		Widget(Widget &&) = delete;
		Widget &operator=(Widget &&) = delete;

		explicit Widget(const Args& args, const Options &options);
		virtual ~Widget() = default;

		// Getters
		[[nodiscard]] Data &data();
		[[nodiscard]] const Data &data() const;
		[[nodiscard]] const std::vector<std::shared_ptr<Widget>> &getChildren() const;
		[[nodiscard]] virtual Rect getRect() const;
		[[nodiscard]] virtual Rect getContentRect() const;
		[[nodiscard]] virtual Rect getLayoutRect() const;
		[[nodiscard]] virtual std::vector<Rect> getHitcheckRect() const;

		// Setters
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