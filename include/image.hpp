#pragma once

#include "engine/pipeline.hpp"
#include "engine/samplerUniform.hpp"
#include "engine/texture.hpp"
#include "engine/texturedQuad.hpp"
#include "vector"
#include "widget.hpp"
#include <future>
#include <memory>
#include <optional>
#include <string_view>
#include <variant>

namespace squi {
	struct Image {
		struct Data {
			std::vector<uint8_t> data;
			int32_t width;
			int32_t height;
			int32_t channels;

			Data(std::vector<uint8_t> data, uint32_t width, uint32_t height, uint32_t channels)
				: data(std::move(data)),
				  width(static_cast<int32_t>(width)),
				  height(static_cast<int32_t>(height)),
				  channels(static_cast<int32_t>(channels)){};
			Data(unsigned char *bytes, uint32_t length);
			static Data fromUrl(std::string_view url);
			static Data fromFile(std::string_view path);
			static std::future<Data> fromUrlAsync(std::string_view url);
			static std::future<Data> fromFileAsync(std::string_view path);

			[[nodiscard]] Engine::Texture createTexture(Engine::Instance &instance) const;
		};

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

		enum class Type {
			normal = 0,
			signedDistanceField = 1,
		};

		// Args
		Widget::Args widget{};
		Fit fit = Fit::none;
		Type type = Type::normal;
		std::variant<Data, std::shared_future<Data>> image;
		using ImagePipeline = Engine::Pipeline<Engine::TexturedQuad::Vertex, true>;

		class Impl : public Widget {
			// Data
			Fit fit;
			Type type = Type::normal;
			std::optional<Engine::SamplerUniform> sampler;
			Engine::TexturedQuad quad{Engine::TexturedQuad::Args{
				.position{0, 0},
				.size{0, 0},
			}};
			std::optional<std::shared_future<Data>> dataFuture{};
			// static std::unique_ptr<ImagePipeline> pipeline;

		public:
			static ImagePipeline *pipeline;
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