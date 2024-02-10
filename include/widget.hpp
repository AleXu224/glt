#pragma once

#include "functional"
#include "margin.hpp"
#include "memory"
#include "rect.hpp"
#include "vec2.hpp"
#include "vector"
#include <any>
#include <functional>
#include <memory>
#include <optional>
#include <type_traits>
#include <unordered_map>
#include <variant>


namespace squi {
	enum class Size {
		// Will expand to fill all the available space
		Expand,
		// Will shrink to the minimum size
		Shrink,
	};

	struct SizeConstraints {
		std::optional<float> minWidth = std::nullopt;
		std::optional<float> minHeight = std::nullopt;
		std::optional<float> maxWidth = std::nullopt;
		std::optional<float> maxHeight = std::nullopt;

		[[nodiscard]] inline SizeConstraints withDefaultMinWidth(float value) const {
			auto copy = *this;
			copy.minWidth = value;
			return copy;
		}

		[[nodiscard]] inline SizeConstraints withDefaultMinHeight(float value) const {
			auto copy = *this;
			copy.minHeight = value;
			return copy;
		}

		[[nodiscard]] inline SizeConstraints withDefaultMaxWidth(float value) const {
			auto copy = *this;
			copy.maxWidth = value;
			return copy;
		}

		[[nodiscard]] inline SizeConstraints withDefaultMaxHeight(float value) const {
			auto copy = *this;
			copy.maxHeight = value;
			return copy;
		}
	};

	class Widget : public std::enable_shared_from_this<Widget> {
	public:
		struct Args {

			/**
			 * @description:
			 * The width of the widget
			 * @note:
			 * The width can be either a float or a Size. If it is a float then the width of
			 * the widget will be considered fixed. Otherwise it will match the selected option.
			 */
			using Width = std::variant<float, Size>;
			std::optional<Width> width{};
			/**
			 * @description:
			 * The height of the widget
			 * @note:
			 * The height can be either a float or a Size. If it is a float then the height of
			 * the widget will be considered fixed. Otherwise it will match the selected option.
			 */
			using Height = std::variant<float, Size>;
			std::optional<Height> height{};
			/**
			 * @brief The maximum and minimum size limits of the widget.
			 */
			SizeConstraints sizeConstraints{};
			/**
			 * The space around the widget. Does not affect the size.
			 */
			std::optional<Margin> margin{};
			/**
			 * The space inside the widget. Does not affect the size.
			 */
			std::optional<Margin> padding{};
			std::function<void(Widget &)> onInit{};
			std::function<void(Widget &)> onUpdate{};
			std::function<void(Widget &)> afterUpdate{};
			std::function<void(Widget &, vec2 &, vec2 &)> beforeLayout{};
			std::function<void(Widget &, vec2 &, vec2 &)> onLayout{};
			std::function<void(Widget &, vec2 &, vec2 &)> afterLayout{};
			std::function<void(Widget &, vec2 &)> onArrange{};
			std::function<void(Widget &)> beforeDraw{};
			std::function<void(Widget &)> onDraw{};
			std::function<void(Widget &)> afterDraw{};
			std::function<void(Widget &, std::shared_ptr<Widget>)> onChildAdded{};
			std::function<void(Widget &, std::shared_ptr<Widget>)> onChildRemoved{};
#ifndef NDEBUG
			std::function<void()> onDebugUpdate{};
			std::function<void()> onDebugLayout{};
			std::function<void()> onDebugArrange{};
			std::function<void()> onDebugDraw{};
#endif

			[[nodiscard]] inline Args withDefaultWidth(const Width &width) const {
				Args args = *this;
				args.width = this->width.value_or(width);
				return args;
			}

			[[nodiscard]] inline Args withDefaultHeight(const Height &height) const {
				Args args = *this;
				args.height = this->height.value_or(height);
				return args;
			}

			[[nodiscard]] inline Args withDefaultMargin(const Margin &margin) const {
				Args args = *this;
				args.margin = this->margin.value_or(margin);
				return args;
			}

			[[nodiscard]] inline Args withDefaultPadding(const Padding &padding) const {
				Args args = *this;
				args.padding = this->padding.value_or(padding);
				return args;
			}

			[[nodiscard]] inline Args withSizeConstraints(const SizeConstraints &sizeConstraints) const {
				Args args = *this;
				args.sizeConstraints = sizeConstraints;
				return args;
			}
		};

		enum class StateImpact {
			None,            // By itself may not change anything but the callback could
			RedrawNeeded,    // Eg: changing the color of a box
			RepositionNeeded,// Changing the scroll on a scrollable
			RelayoutNeeded,  // Size change
		};

		template<class T, StateImpact stateImpact>
		struct Stateful {
			template<class... Args>
			Stateful(Widget *parent, const Args &...args) : item(args...), parent(parent) {}
			template<class... Args>
			Stateful(Widget *parent, std::function<void(const T &)> callback, const Args &...args) : item(args...), parent(parent), callback(callback) {}

			Stateful(const Stateful &) = delete;
			Stateful(Stateful &) = delete;
			Stateful(const Stateful &&) = delete;
			Stateful(Stateful &&) = delete;

			inline operator const Stateful &() const {
				return item;
			}

			inline Stateful &operator=(const T &other) {
				if (item != other) {
					item = other;
					if constexpr (stateImpact == StateImpact::RedrawNeeded) {
						parent->reDraw();
					} else if constexpr (stateImpact == StateImpact::RepositionNeeded) {
						parent->reArrange();
					} else if constexpr (stateImpact == StateImpact::RelayoutNeeded) {
						parent->reLayout();
					}
				}
				return *this;
			}

			// template<class Q = T, std::enable_if_t<std::is_pointer<Q>::value, bool> = true>
			inline const T &operator->() const requires(std::is_pointer_v<T>) {
				return item;
			}

			// template<class Q = T, std::enable_if_t<!std::is_pointer<Q>::value, bool> = true>
			inline const T *operator->() const requires(!std::is_pointer_v<T>) {
				return std::addressof(item);
			}

			inline const T &operator*() const {
				return item;
			}

		private:
			T item;
			Widget *parent;
			std::function<void(const T&)> callback{};
		};

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
			std::vector<std::function<void(Widget &)>> onUpdate{};
			std::vector<std::function<void(Widget &)>> afterUpdate{};
			std::vector<std::function<void(Widget &, vec2 &, vec2 &)>> beforeLayout{};
			std::vector<std::function<void(Widget &, vec2 &, vec2 &)>> onLayout{};
			std::vector<std::function<void(Widget &, vec2 &, vec2 &)>> afterLayout{};
			std::vector<std::function<void(Widget &, vec2 &)>> onArrange{};
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
		static uint64_t idCounter;
		static uint32_t widgetCount;

	public:
		struct State {
			std::unordered_map<std::string_view, std::any> properties{};
			Stateful<std::variant<float, Size>, StateImpact::RelayoutNeeded> width;
			Stateful<std::variant<float, Size>, StateImpact::RelayoutNeeded> height;
			// FIXME: Also make this const and add a setter
			SizeConstraints sizeConstraints;
			Stateful<Margin, StateImpact::RelayoutNeeded> margin;
			Stateful<Margin, StateImpact::RelayoutNeeded> padding;
			Stateful<Widget *, StateImpact::RelayoutNeeded> parent;
			Stateful<Widget *, StateImpact::RelayoutNeeded> root;
		};
		State state;
		const uint64_t id;
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
		[[nodiscard]] inline Rect getRect() const {
			return Rect::fromPosSize(pos + state.margin->getPositionOffset(), size);
		}
		[[nodiscard]] inline Rect getContentRect() const {
			return Rect::fromPosSize(
				pos + state.margin->getPositionOffset() + state.padding->getPositionOffset(),
				size - state.padding->getSizeOffset());
		}
		[[nodiscard]] inline Rect getLayoutRect() const {
			return Rect::fromPosSize(
				pos,
				size + state.margin->getSizeOffset());
		}
		[[nodiscard]] inline vec2 getSize() const {
			return size;
		}
		[[nodiscard]] inline vec2 getContentSize() const {
			return size - state.padding->getSizeOffset();
		}
		[[nodiscard]] inline vec2 getLayoutSize() const {
			return size + state.margin->getSizeOffset();
		}
		[[nodiscard]] inline vec2 getPos() const {
			return pos;
		}
		[[nodiscard]] inline vec2 getContentPos() const {
			return pos + state.margin->getPositionOffset() + state.padding->getPositionOffset();
		}
		[[nodiscard]] virtual std::vector<Rect> getHitcheckRect() const;
		[[nodiscard]] inline bool isMarkedForDeletion() const {
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
		vec2 layout(vec2 maxSize, vec2 minSize, ShouldShrink forceShrink = {false, false});
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
		inline void deleteLater() {
			shouldDelete = true;
		}

		// Virtual methods
		virtual void onUpdate() {};
		virtual void updateChildren();
		virtual void afterUpdate() {};

		virtual void onLayout(vec2 &maxSize, vec2 &minSize) {};
		virtual vec2 layoutChildren(vec2 maxSize, vec2 minSize, ShouldShrink shouldShrink);
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

	using Child = std::shared_ptr<Widget>;
	using ChildRef = std::weak_ptr<Widget>;
	using Children = std::vector<Child>;
}// namespace squi