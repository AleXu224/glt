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
#include "gestureDetector.hpp"

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
			std::function<void(Widget &)> beforeDraw{};
			std::function<void(Widget &)> onDraw{};
			std::function<void(Widget &)> afterDraw{};
		};

		struct Options {
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
			bool isInteractive = false;

			static Options Default() {
				return Options();
			}
		};

	private:
		bool isInitialized = false;
		bool shouldUpdateChildren;
		bool shouldDrawChildren;
		bool shouldHandleSizeBehavior;
		bool isInteractive;
		struct FunctionArgs {
			std::function<void(Widget &)> onInit;
			std::function<void(Widget &)> beforeUpdate;
			std::function<void(Widget &)> onUpdate;
			std::function<void(Widget &)> afterUpdate;
			std::function<void(Widget &)> beforeDraw;
			std::function<void(Widget &)> onDraw;
			std::function<void(Widget &)> afterDraw;
		};
		FunctionArgs m_funcs;
		struct Data {
			vec2 size;
			Margin margin;
			Margin padding;
			SizeBehavior sizeBehavior;
			vec2 pos{};
			vec2 sizeHint{-1, -1};
			Widget *parent = nullptr;
			GestureDetector gestureDetector;
			bool visible = true;

			void print() const {
				printf("Size: %f, %f\n", size.x, size.y);
				printf("Margin: %f, %f, %f, %f\n", margin.left, margin.top, margin.right, margin.bottom);
				printf("Padding: %f, %f, %f, %f\n", padding.left, padding.top, padding.right, padding.bottom);
				printf("Size Behavior: %d, %d\n", sizeBehavior.horizontal, sizeBehavior.vertical);
				printf("Pos: %f, %f\n", pos.x, pos.y);
				printf("Size Hint: %f, %f\n", sizeHint.x, sizeHint.y);
				printf("Parent: %p\n", parent);
				printf("Visible: %d\n", visible);
			}
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
		[[nodiscard]] FunctionArgs &funcs();
		[[nodiscard]] const FunctionArgs &funcs() const;
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
		virtual void fillParentSizeBehavior(bool horizontalHint, bool verticalHint);
		virtual void matchChildSizeBehavior(bool horizontalHint, bool verticalHint);
		virtual void onUpdate(){};
		virtual void afterChildrenUpdate(){};
		virtual void afterUpdate(){};
		virtual void onDraw(){};
	};
}// namespace squi

#endif