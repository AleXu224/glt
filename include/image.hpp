#pragma once

#include <memory>

#include "engine/pipeline.hpp"
#include "engine/samplerUniform.hpp"
#include "engine/texturedQuad.hpp"
#include "image/provider.hpp"
#include "widget.hpp"

namespace squi {
	struct Image {
		enum class Fit {
			// The image will be displayed at its original size.
			none,
			// The image will be resized and stretched to fill the widget.
			fill,
			// The image will be resized to fill the widget while maintaining the aspect ratio.
			cover,
			// The image will be resized to completely fit inside the widget while maintaining the aspect ratio.
			contain,
		};

		// Args
		Widget::Args widget{};
		Fit fit = Fit::none;
		// FIXME: add the ability to provide a sampler shared reference
		ImageProvider image;
		using ImagePipeline = Engine::Pipeline<Engine::TexturedQuad::Vertex, true>;

		class Impl : public Widget {
			// Data
			Fit fit;
			Engine::TexturedQuad quad{Engine::TexturedQuad::Args{
				.position{0, 0},
				.size{0, 0},
			}};
			std::shared_ptr<Engine::SamplerUniform> sampler;
			bool relayoutNextFrame = false;

		public:
			// static ImagePipeline *pipeline;
			std::shared_ptr<ImagePipeline> pipeline;
			Impl(const Image &args);

			void onUpdate() override;
			void onLayout(vec2 &maxSize, vec2 &minSize) override;
			void postLayout(vec2 &size) override;
			void postArrange(vec2 &pos) override;
			void onDraw() override;

			[[nodiscard]] Engine::TexturedQuad &getQuad() {
				return quad;
			}
		};

		operator Child() const {
			return std::make_shared<Impl>(*this);
		}
	};
}// namespace squi