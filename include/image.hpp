#pragma once

#include "quad.hpp"
#include "texture.hpp"
#include "widget.hpp"
#include "vector"
#include <memory>
#include <optional>
#include <stdio.h>
#include <string_view>
#include <thread>
#include <unordered_map>

namespace squi {
    struct Image {
		struct Data {
			std::vector<uint8_t> data;
			uint32_t width;
			uint32_t height;
			uint32_t channels;
            bool ready = false;

            Data() : width(0), height(0), channels(0) {}
            Data(unsigned char *bytes, uint32_t length);
            static Data fromUrl(std::string_view url);
			static Data fromFile(std::string_view path);
			static Data fromUrlAsync(std::string_view url);
			// static std::thread fromFileAsync(std::string_view path, std::function<void(Data)>);

            std::shared_ptr<Data> loaderData;

			[[nodiscard]] Texture::Impl createTexture() const;
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

		// Args
        Widget::Args widget;
        Fit fit = Fit::none;
        Data image;
    
        class Impl : public Widget {
            // Data
            Texture::Impl texture;
			Fit fit;
			float aspectRatio;
			uint32_t width;
			uint32_t height;
            std::shared_ptr<Data> tempData{};
            Quad quad;
    
        public:
            Impl(const Image &args);

            void onUpdate() override;
            void onLayout(vec2 &maxSize, vec2 &minSize) override;
            void postLayout(vec2 &size) override;
            void postArrange(vec2 &pos) override;
            void onDraw() override;
        };
    
        operator Child() const {
            return std::make_shared<Impl>(*this);
        }
    };
}