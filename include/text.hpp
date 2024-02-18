#pragma once

#include "color.hpp"
#include "fontStore.hpp"
#include "engine/pipeline.hpp"
#include "engine/textQuad.hpp"
#include "vec2.hpp"
#include "widget.hpp"
#include <memory>
#include <string_view>


namespace squi {
	struct Text {
		// Args
		Widget::Args widget{};
		std::string_view text;
		float fontSize{14.0f};
		bool lineWrap{false};
		std::variant<std::string_view, std::shared_ptr<FontStore::Font>> font = R"(C:\Windows\Fonts\arial.ttf)";
		Color color{0xFFFFFFFF};
		using TextPipeline = Engine::Pipeline<Engine::TextQuad::Vertex, true>;

		class Impl : public Widget {
			// Data
			float lastX{0};
			float lastY{0};
			float lastAvailableSpace{0};
			std::string text;
			std::string fontSrc;
			float fontSize;
			bool lineWrap;
			bool textModified = false;
			std::optional<std::shared_ptr<FontStore::Font>> font{};
			Color color;
			vec2 textSize;
			std::vector<std::vector<Engine::TextQuad>> quads{};
			// static std::unique_ptr<TextPipeline> pipeline;
			static TextPipeline *pipeline;

		public:
			Impl(const Text &args);

			vec2 layoutChildren(vec2 maxSize, vec2 minSize, ShouldShrink shouldShrink) final;
			void onArrange(vec2 &pos) final;

			void updateSize();

			void onDraw() final;

			void setText(const std::string_view &text);

			[[nodiscard]] std::tuple<uint32_t, uint32_t> getTextSize(const std::string_view &text) const;

			[[nodiscard]] std::string_view getText() const;
		};

		operator Child() const {
			return std::make_shared<Impl>(*this);
		}
	};
}// namespace squi