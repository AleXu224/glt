#ifndef SQUI_TEXTURE_HPP
#define SQUI_TEXTURE_HPP

#include "d3d11.h"
#include "memory"

namespace squi {
    struct Texture {
        uint16_t width;
        uint16_t height;
        uint16_t channels = 4;
        std::shared_ptr<unsigned char> data;
        bool dynamic = false;

        class Impl {
            std::shared_ptr<ID3D11Texture2D> texture{};
            std::shared_ptr<ID3D11ShaderResourceView> textureView{};

        public:
            Impl(const Texture &texture);

            [[nodiscard]] const std::shared_ptr<ID3D11ShaderResourceView> &getTextureView() const;
        };

        operator Impl() const;
    };
}

#endif