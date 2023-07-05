#ifndef SQUI_TEXTURE_HPP
#define SQUI_TEXTURE_HPP

#include "d3d11.h"
#include "memory"

namespace squi {
	struct Texture {
		const uint16_t width = 0;
		const uint16_t height = 0;
		const uint16_t channels = 4;
		const uint8_t *data = nullptr;
		bool dynamic = false;

		struct Properties {
			uint16_t width;
			uint16_t height;
			uint16_t channels;
			bool dynamic;
		};

		class Impl {
			std::shared_ptr<ID3D11Texture2D> texture{};
			std::shared_ptr<ID3D11ShaderResourceView> textureView{};
			Properties properties;

		public:
			Impl(const Texture &args);

			[[nodiscard]] const Properties &getProperties() const;
			[[nodiscard]] const std::shared_ptr<ID3D11ShaderResourceView> &getTextureView() const;
		};

		[[nodiscard]] static Impl Empty();

		operator Impl() const;
	};
}// namespace squi

#endif