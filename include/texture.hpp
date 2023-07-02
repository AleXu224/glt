#ifndef SQUI_TEXTURE_HPP
#define SQUI_TEXTURE_HPP

#include "d3d11.h"
#include "memory"

namespace squi {
    struct Texture {
        uint16_t width = 0;
        uint16_t height = 0;
        uint16_t channels = 4;
        const uint8_t *data = nullptr;
        bool dynamic = false;

        class Impl {
            std::shared_ptr<ID3D11Texture2D> texture{};
            std::shared_ptr<ID3D11ShaderResourceView> textureView{};

        public:
            Impl(const Texture &args);

            [[nodiscard]] const std::shared_ptr<ID3D11ShaderResourceView> &getTextureView() const;
        };

        operator Impl() const;
    };
}

#endif