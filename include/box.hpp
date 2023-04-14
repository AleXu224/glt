#ifndef SQUI_BOX_HPP
#define SQUI_BOX_HPP

#include "color.hpp"
#include "quad.hpp"
#include "widget.hpp"

namespace squi {
	struct Box {
		enum class BorderPosition {
			inset,
			outset,
		};
		
		Widget::Args widget;
		Color color{Color::HEX(0xFFFFFFFF)};
		Color borderColor{Color::HEX(0x000000FF)};
		float borderWidth{0.0f};
		float borderRadius{0.0f};
		BorderPosition borderPosition{BorderPosition::inset};
		bool shouldClipContent = true;
		Child child;

		class Impl : public Widget {
			Quad quad;
			// This is stored as a hack to get around the fact the renderer doesn't support
			// overlapping the border color on top of the background color.
			Color borderColor;
			BorderPosition borderPosition;
			bool shouldClipContent;

		public:
			Impl(const Box &args);

			void onDraw() override;

			void setColor(const Color &color);
			void setBorderColor(const Color &color);
			void setBorderWidth(float width);
			void setBorderRadius(float radius);

			[[nodiscard]] Color getColor() const;
			[[nodiscard]] Color getBorderColor() const;
			[[nodiscard]] float getBorderWidth() const;
			[[nodiscard]] float getBorderRadius() const;
			[[nodiscard]] Quad &getQuad();
		};

		operator std::shared_ptr<Widget>() {
			return std::make_shared<Impl>(*this);
		}

		operator Child() {
			return Child(std::make_shared<Impl>(*this));
		}
	};
}// namespace squi

#endif//SQUI_BOX_HPP
