#pragma once

#include "color.hpp"
#include "quad.hpp"
#include "widget.hpp"
#include <memory>

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
			BorderPosition borderPosition;
			bool shouldClipContent;

		public:
			explicit Impl(const Box &args);

			void onDraw() final;
			void drawChildren() final;

			void postLayout(vec2 &size) final;
			void postArrange(vec2 &pos) final;

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

		operator Child() {
			return {std::make_shared<Impl>(*this)};
		}
	};
}// namespace squi
