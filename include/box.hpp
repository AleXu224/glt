#pragma once

#include "color.hpp"
#include "widget.hpp"
#include "engine/quad.hpp"
#include "engine/pipeline.hpp"
#include <memory>

namespace squi {
	struct Box {
		enum class BorderPosition {
			inset,
			outset,
		};

		Widget::Args widget{};
		Color color{0xFFFFFFFF};
		Color borderColor{0x000000FF};
		glm::vec4 borderWidth{0.0f};
		glm::vec4 borderRadius{0.0f};
		BorderPosition borderPosition{BorderPosition::inset};
		bool shouldClipContent = true;
		Child child{};
		using BoxPipeline = Engine::Pipeline<Engine::Quad::Vertex>;
		class Impl : public Widget {
			Engine::Quad quad;
			// This is stored as a hack to get around the fact the renderer doesn't support
			// overlapping the border color on top of the background color.
			BorderPosition borderPosition;
			bool shouldClipContent;
			// static std::unique_ptr<BoxPipeline> pipeline;
			static BoxPipeline *pipeline;

		public:
			explicit Impl(const Box &args);

			void onDraw() final;
			void drawChildren() final;

			void postLayout(vec2 &size) final;
			void postArrange(vec2 &pos) final;

			void setColor(const Color &color);
			void setBorderColor(const Color &color);
			void setBorderWidth(glm::vec4 width);
			void setBorderRadius(glm::vec4 radius);

			[[nodiscard]] Color getColor() const;
			[[nodiscard]] Color getBorderColor() const;
			[[nodiscard]] glm::vec4 getBorderWidth() const;
			[[nodiscard]] glm::vec4 getBorderRadius() const;
			[[nodiscard]] Engine::Quad &getQuad();
		};

		operator Child() {
			return {std::make_shared<Impl>(*this)};
		}
	};
}// namespace squi
