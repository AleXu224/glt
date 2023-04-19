#ifndef SQUI_WIDGEt_HPP
#define SQUI_WIDGEt_HPP

#include "child.hpp"
#include "functional"
#include "margin.hpp"
#include "memory"
#include "rect.hpp"
#include "vec2.hpp"
#include "vector"
#include <optional>
#include <stdint.h>
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
	};

	class Widget {
	public:
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
			bool shouldHandleLayout = true;
			/**
			 * Set to false to manually position the children
			 */
			bool shouldArrangeChildren = true;
			/**
			 * Whether hit testing should be performed on this widget
			 */
			bool isInteractive = false;

			static Options Default() {
				return {};
			}
		};

	private:
		bool isInitialized = false;
		bool shouldUpdateChildren;
		bool shouldDrawChildren;
		bool shouldHandleLayout;
		bool shouldArrangeChildren;
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
		struct Data {
			struct SizeMode {
				std::variant<float, Size> width;
				std::variant<float, Size> height;
			};
			SizeMode sizeMode;
			SizeConstraints sizeConstraints;
			Margin margin;
			Margin padding;
			Widget *parent = nullptr;
			bool visible = true;
			bool isInteractive;

			void print() const {
				printf("Margin: %f, %f, %f, %f\n", margin.left, margin.top, margin.right, margin.bottom);
				printf("Padding: %f, %f, %f, %f\n", padding.left, padding.top, padding.right, padding.bottom);
				printf("Parent: %p\n", parent);
				printf("Visible: %d\n", visible);
				printf("Is Interactive: %d\n", isInteractive);
			}
		};
		Data m_data;
		std::vector<std::shared_ptr<Widget>> children{};
		static uint32_t widgetCount;

	public:
		// Disable copy
		Widget(const Widget &) = delete;
		Widget &operator=(const Widget &) = delete;
		// Disable move
		Widget(Widget &&) = delete;
		Widget &operator=(Widget &&) = delete;

		explicit Widget(const Args &args, const Options &options);
		virtual ~Widget();

		// Getters
		[[nodiscard]] Data &data();
		[[nodiscard]] const Data &data() const;
		[[nodiscard]] FunctionArgs &funcs();
		[[nodiscard]] const FunctionArgs &funcs() const;
		[[nodiscard]] const std::vector<std::shared_ptr<Widget>> &getChildren() const;
		[[nodiscard]] inline Rect getRect() const {
			return Rect::fromPosSize(pos + m_data.margin.getPositionOffset(), size);
		}
		[[nodiscard]] inline Rect getContentRect() const {
			return Rect::fromPosSize(
				pos + m_data.margin.getPositionOffset() + m_data.padding.getPositionOffset(),
				size - m_data.padding.getSizeOffset());
		}
		[[nodiscard]] inline Rect getLayoutRect() const {
			return Rect::fromPosSize(
				pos,
				size + m_data.margin.getSizeOffset());
		}
		[[nodiscard]] inline vec2 getSize() const {
			return size;
		}
		[[nodiscard]] inline vec2 getContentSize() const {
			return size - m_data.padding.getSizeOffset();
		}
		[[nodiscard]] inline vec2 getLayoutSize() const {
			return size + m_data.margin.getSizeOffset();
		}
		[[nodiscard]] inline vec2 getPos() const {
			return pos;
		}
		[[nodiscard]] inline vec2 getContentPos() {
			return pos + m_data.margin.getPositionOffset() + m_data.padding.getPositionOffset();
		}
		[[nodiscard]] virtual std::optional<Rect> getHitcheckRect() const;

		// Setters
		void setChildren(const std::vector<std::shared_ptr<Widget>> &newChildren);

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

		// Virtual methods
		virtual void init(){};
		virtual void onUpdate(){};
		virtual void afterUpdate(){};
		virtual void onLayout(vec2 &maxSize, vec2 &minSize){};
		virtual void postLayout(vec2 size){};
		virtual void onArrange(vec2 &pos){};
		virtual void postArrange(vec2 pos){};
		virtual void onDraw(){};
	};
}// namespace squi

#endif