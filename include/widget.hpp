#ifndef SQUI_WIDGEt_HPP
#define SQUI_WIDGEt_HPP

#include "child.hpp"
#include "functional"
#include "margin.hpp"
#include "memory"
#include "rect.hpp"
#include "vec2.hpp"
#include "vector"
#include <any>
#include <optional>
#include <unordered_map>
#include <variant>
#include "string"

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
	};

	class Widget : public std::enable_shared_from_this<Widget> {
	public:
		// struct Store {
		// 	static std::unordered_map<uint64_t, ChildRef> widgets;

		// 	static Child getWidget(uint64_t id);
		// };
		struct Args {

			/**
			 * @description:
			 * The width of the widget
			 * @note:
			 * The width can be either a float or a Size. If it is a float then the width of
			 * the widget will be considered fixed. Otherwise it will match the selected option.
			 */
			std::variant<float, Size> width = Size::Expand;
			/**
			 * @description:
			 * The height of the widget
			 * @note:
			 * The height can be either a float or a Size. If it is a float then the height of
			 * the widget will be considered fixed. Otherwise it will match the selected option.
			 */
			std::variant<float, Size> height = Size::Expand;
			/**
			 * @brief The maximum and minimum size limits of the widget.
			 */
			SizeConstraints sizeConstraints{};
			/**
			 * The space around the widget. Does not affect the size.
			 */
			Margin margin;
			/**
			 * The space inside the widget. Does not affect the size.
			 */
			Margin padding;
			std::function<void(Widget &)> onInit{};
			std::function<void(Widget &)> onUpdate{};
			std::function<void(Widget &)> afterUpdate{};
			std::function<void(Widget &, vec2 &, vec2 &)> onLayout{};
			std::function<void(Widget &, vec2 &)> onArrange{};
			std::function<void(Widget &)> beforeDraw{};
			std::function<void(Widget &)> onDraw{};
			std::function<void(Widget &)> afterDraw{};
		};

		struct Flags {
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
			bool isInteractive = false;
			bool visible = true;

			static Flags Default() {
				return {};
			}
		};

		Flags flags;
	private:
		bool shouldDelete = false;
		struct FunctionArgs {
			std::vector<std::function<void(Widget &)>> onInit{};
			std::vector<std::function<void(Widget &)>> onUpdate{};
			std::vector<std::function<void(Widget &)>> afterUpdate{};
			std::vector<std::function<void(Widget &, vec2 &, vec2 &)>> onLayout{};
			std::vector<std::function<void(Widget &, vec2 &)>> onArrange{};
			std::vector<std::function<void(Widget &)>> beforeDraw{};
			std::vector<std::function<void(Widget &)>> onDraw{};
			std::vector<std::function<void(Widget &)>> afterDraw{};
		};
		FunctionArgs m_funcs{};
		vec2 size{};
		vec2 pos{};
		std::vector<Child> children{};
		static uint64_t idCounter;
		static uint32_t widgetCount;

	public:
		struct State {
			struct SizeMode {
				std::variant<float, Size> width;
				std::variant<float, Size> height;
			};
			std::unordered_map<std::string_view, std::any> properties{};
			SizeMode sizeMode;
			SizeConstraints sizeConstraints;
			Margin margin;
			Margin padding;
			Widget *parent = nullptr;
		};
		State state;
		const uint64_t id;
		// Disable copy
		Widget(const Widget &) = delete;
		Widget &operator=(const Widget &) = delete;
		// Disable move
		Widget(Widget &&) = delete;
		Widget &operator=(Widget &&) = delete;

		explicit Widget(const Args &args, const Flags &flags);
		virtual ~Widget();
		[[nodiscard]] FunctionArgs &funcs();
		[[nodiscard]] const FunctionArgs &funcs() const;
		[[nodiscard]] std::vector<Child> &getChildren();
		[[nodiscard]] inline Rect getRect() const {
			return Rect::fromPosSize(pos + state.margin.getPositionOffset(), size);
		}
		[[nodiscard]] inline Rect getContentRect() const {
			return Rect::fromPosSize(
				pos + state.margin.getPositionOffset() + state.padding.getPositionOffset(),
				size - state.padding.getSizeOffset());
		}
		[[nodiscard]] inline Rect getLayoutRect() const {
			return Rect::fromPosSize(
				pos,
				size + state.margin.getSizeOffset());
		}
		[[nodiscard]] inline vec2 getSize() const {
			return size;
		}
		[[nodiscard]] inline vec2 getContentSize() const {
			return size - state.padding.getSizeOffset();
		}
		[[nodiscard]] inline vec2 getLayoutSize() const {
			return size + state.margin.getSizeOffset();
		}
		[[nodiscard]] inline vec2 getPos() const {
			return pos;
		}
		[[nodiscard]] inline vec2 getContentPos() {
			return pos + state.margin.getPositionOffset() + state.padding.getPositionOffset();
		}
		[[nodiscard]] virtual std::optional<Rect> getHitcheckRect() const;
		[[nodiscard]] inline bool isMarkedForDeletion() const {
			return shouldDelete;
		}

		// Setters
		void setChildren(const Children &newChildren);

		// Methods
		void addChild(const Child &child);
		void update();
		/**
		 * @brief Layout the widget
		 * 
		 * @param maxSize The maximum size the total size of the widget can be
		 * @return vec2 The minimum size the content of the widget needs to be
		 */
		vec2 layout(vec2 maxSize);
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
		virtual void onUpdate(){};
		virtual void updateChildren();
		virtual void afterUpdate(){};

		virtual float getMinWidth();
		virtual float getMinHeight();

		virtual void onLayout(vec2 &maxSize, vec2 &minSize){};
		virtual void layoutChildren(vec2& maxSize, vec2& minSize);
		virtual void postLayout(vec2 &size){};

		virtual void onArrange(vec2 &pos){};
		virtual void arrangeChildren(vec2 &pos);
		virtual void postArrange(vec2 &pos){};

		virtual void onDraw(){};
		virtual void drawChildren();
	};
}// namespace squi

#endif