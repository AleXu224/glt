#pragma once

#include "functional"
#include "memory"
#include "rect.hpp"
#include "stateContainer.hpp"
#include "stateful.hpp"
#include "vec2.hpp"
#include "vector"
#include "widgetArgs.hpp"
#include <atomic>

namespace squi {
	class Window;

	class Widget : public std::enable_shared_from_this<Widget> {
	public:
		friend Window;
		using Args = ::squi::Args;

		struct FlagsArgs {
			/**
			 * Set to false to disable updating the children
			 */
			bool shouldUpdateChildren = true;
			/**
			 * Set to false to disable drawing the children
			 */
			bool shouldDrawChildren = true;
			/**
			 * Set to false to disable sizing the children
			 */
			bool shouldLayoutChildren = true;
			/**
			 * Set to false to disable positioning the children
			 */
			bool shouldArrangeChildren = true;
			/**
			 * Whether hit testing should be performed on this widget
			 */
			bool isInteractive = true;
			bool visible = true;

			static FlagsArgs Default() {
				return {};
			}
		};
		struct Flags {
			bool shouldUpdateChildren;
			Stateful<bool, StateImpact::RedrawNeeded> shouldDrawChildren;
			bool shouldLayoutChildren;
			bool shouldArrangeChildren;
			bool isInteractive;
			Stateful<bool, StateImpact::RelayoutNeeded> visible;

			Flags(Widget *w, const FlagsArgs &args)
				: shouldUpdateChildren(args.shouldUpdateChildren),
				  shouldDrawChildren(w, args.shouldDrawChildren),
				  shouldLayoutChildren(args.shouldLayoutChildren),
				  shouldArrangeChildren(args.shouldArrangeChildren),
				  isInteractive(args.isInteractive),
				  visible(w, args.visible) {}
		} flags;

	private:
		bool shouldDelete = false;
		bool initialized = false;
		struct FunctionArgs {
			std::vector<std::function<void(Widget &)>> onInit{};
			std::vector<std::function<void(Widget &)>> afterInit{};
			std::vector<std::function<void(Widget &)>> onUpdate{};
			std::vector<std::function<void(Widget &)>> afterUpdate{};
			std::vector<std::function<void(Widget &, vec2 &, vec2 &)>> beforeLayout{};
			std::vector<std::function<void(Widget &, vec2 &, vec2 &)>> onLayout{};
			std::vector<std::function<void(Widget &, vec2 &)>> afterLayout{};
			std::vector<std::function<void(Widget &, vec2 &)>> onArrange{};
			std::vector<std::function<void(Widget &, vec2 &)>> afterArrange{};
			std::vector<std::function<void(Widget &)>> beforeDraw{};
			std::vector<std::function<void(Widget &)>> onDraw{};
			std::vector<std::function<void(Widget &)>> afterDraw{};
			std::vector<std::function<void(Widget &, std::shared_ptr<Widget>)>> onChildAdded{};
			std::vector<std::function<void(Widget &, std::shared_ptr<Widget>)>> onChildRemoved{};
#ifndef NDEBUG
			std::vector<std::function<void()>> onDebugUpdate{};
			std::vector<std::function<void()>> onDebugLayout{};
			std::vector<std::function<void()>> onDebugArrange{};
			std::vector<std::function<void()>> onDebugDraw{};
#endif
		};
		FunctionArgs m_funcs{};
		vec2 size{};
		vec2 pos{};
		std::vector<std::shared_ptr<Widget>> children{};
		std::vector<std::shared_ptr<Widget>> childrenToAdd{};
		static inline std::atomic<uint64_t> idCounter = 1;
		static inline std::atomic<uint32_t> widgetCount = 0;

		void insertChildren();

	public:
		static uint32_t getCount() {
			return widgetCount;
		}
		CustomState customState{};
		struct State {
			Stateful<std::variant<float, Size>, StateImpact::RelayoutNeeded> width;
			Stateful<std::variant<float, Size>, StateImpact::RelayoutNeeded> height;
			// FIXME: Also make this const and add a setter
			SizeConstraints sizeConstraints;
			Stateful<Margin, StateImpact::RelayoutNeeded> margin;
			Stateful<Margin, StateImpact::RelayoutNeeded> padding;
			Stateful<Widget *, StateImpact::RelayoutNeeded> parent;
			Stateful<Widget *, StateImpact::RelayoutNeeded> root;
		};
		const uint64_t id;
		State state;
		// Disable copy
		Widget(const Widget &) = delete;
		Widget &operator=(const Widget &) = delete;
		// Disable move
		Widget(Widget &&) = delete;
		Widget &operator=(Widget &&) = delete;

		explicit Widget(const Args &args, const FlagsArgs &flags);
		virtual ~Widget();

		[[nodiscard]] FunctionArgs &funcs();
		[[nodiscard]] const FunctionArgs &funcs() const;
		[[nodiscard]] std::vector<std::shared_ptr<Widget>> &getChildren();
		[[nodiscard]] const std::vector<std::shared_ptr<Widget>> &getChildren() const;
		[[nodiscard]] std::vector<std::shared_ptr<Widget>> &getPendingChildren() {
			return childrenToAdd;
		}
		[[nodiscard]] Rect getRect() const {
			return Rect::fromPosSize(pos + state.margin->getPositionOffset(), size);
		}
		[[nodiscard]] Rect getContentRect() const {
			return Rect::fromPosSize(
				pos + state.margin->getPositionOffset() + state.padding->getPositionOffset(),
				size - state.padding->getSizeOffset()
			);
		}
		[[nodiscard]] Rect getLayoutRect() const {
			return Rect::fromPosSize(
				pos,
				size + state.margin->getSizeOffset()
			);
		}
		[[nodiscard]] vec2 getSize() const {
			return size;
		}
		[[nodiscard]] vec2 getContentSize() const {
			return size - state.padding->getSizeOffset();
		}
		[[nodiscard]] vec2 getLayoutSize() const {
			return size + state.margin->getSizeOffset();
		}
		[[nodiscard]] vec2 getPos() const {
			return pos;
		}
		[[nodiscard]] vec2 getContentPos() const {
			return pos + state.margin->getPositionOffset() + state.padding->getPositionOffset();
		}
		[[nodiscard]] virtual std::vector<Rect> getHitcheckRect() const;
		[[nodiscard]] bool isMarkedForDeletion() const {
			return shouldDelete;
		}

		template<class T>
		T &as() {
			return dynamic_cast<T &>(*this);
		}

		// Setters
		void setChildren(const std::vector<std::shared_ptr<Widget>> &newChildren);

		// Methods
		void addChild(const std::shared_ptr<Widget> &child);
		void addOverlay(const std::shared_ptr<Widget> &child);
		void update();
		struct ShouldShrink {
			bool width = false;
			bool height = false;
		};
		/**
		 * @brief Layout the widget
		 * 
		 * @param maxSize The maximum size the total size of the widget can be
		 * @return vec2 The minimum size the content of the widget needs to be
		 */
		vec2 layout(vec2 maxSize, vec2 minSize, ShouldShrink forceShrink, bool final);
		/**
		 * @brief Arrange the widget
		 * 
		 * @param pos The top-left most position the widget can be placed at
		 */
		void arrange(vec2 pos);
		void draw();
		void initialize();
		/**
		 * @brief Mark the widget for deletion
		 */
		void deleteLater() {
			shouldDelete = true;
		}

		// Virtual methods
		virtual void onUpdate() {};
		virtual void updateChildren();
		virtual void afterUpdate() {};

		virtual void onLayout(vec2 &maxSize, vec2 &minSize) {};
		virtual vec2 layoutChildren(vec2 maxSize, vec2 minSize, ShouldShrink shouldShrink, bool final);
		virtual void postLayout(vec2 &size) {};

		virtual void onArrange(vec2 &pos) {};
		virtual void arrangeChildren(vec2 &pos);
		virtual void postArrange(vec2 &pos) {};

		virtual void onDraw() {};
		virtual void drawChildren();

		void reDraw() const;
		void reLayout() const;
		void reArrange() const;
	};
}// namespace squi