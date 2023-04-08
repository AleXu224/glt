#ifndef SQUI_BOX_HPP
#define SQUI_BOX_HPP

#include "color.hpp"
#include "quad.hpp"
#include "widget.hpp"

namespace squi {
	struct Box {
		Widget::Args widget;
		Color color{Color::HEX(0xFFFFFFFF)};
		Color borderColor{Color::HEX(0x000000FF)};
		float borderWidth{0.0f};
		float borderRadius{0.0f};
		Child child;

		class Impl : public Widget {
			Quad quad;

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
