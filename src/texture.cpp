#include "texture.hpp"
#include "renderer.hpp"
#include "stdexcept"


using namespace squi;

constexpr uint8_t emptyData[4] = {0, 0, 0, 0};

Texture::Impl::Impl(const Texture &args) : properties(Properties{
											   .width = args.width,
											   .height = args.height,
											   .channels = args.channels,
											   .dynamic = args.dynamic,
										   }) {
	D3D11_TEXTURE2D_DESC textureDesc{};
	textureDesc.Width = args.width;
	textureDesc.Height = args.height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	DXGI_FORMAT format;
	switch (args.channels) {
		case 1:
			format = DXGI_FORMAT_R8_UNORM;
			break;
		case 2:
			format = DXGI_FORMAT_R8G8_UNORM;
			break;
		case 3:
			throw std::runtime_error("3 channel textures are not supported");
		case 4:
			format = DXGI_FORMAT_R8G8B8A8_UNORM;
			break;
		default:
			throw std::runtime_error("Invalid number of channels");
	}
	textureDesc.Format = format;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = args.dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = args.dynamic ? D3D11_CPU_ACCESS_WRITE : 0;
	textureDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA subresourceData{};
	subresourceData.pSysMem = args.data;
	subresourceData.SysMemPitch = args.width * args.channels;

	const auto &instance = Renderer::getInstance();
	auto device = instance.getDevice();
	ID3D11Texture2D *texturePtr;
	device->CreateTexture2D(&textureDesc, &subresourceData, &texturePtr);
	texture = std::shared_ptr<ID3D11Texture2D>(texturePtr, [](ID3D11Texture2D *tex) { tex->Release(); });

	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc{};
	shaderResourceViewDesc.Format = textureDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	ID3D11ShaderResourceView *textureViewPtr;
	device->CreateShaderResourceView(texturePtr, &shaderResourceViewDesc, &textureViewPtr);
	textureView = std::shared_ptr<ID3D11ShaderResourceView>(textureViewPtr, [](ID3D11ShaderResourceView *srv) { srv->Release(); });
}

const Texture::Properties &Texture::Impl::getProperties() const {
	return properties;
}

const std::shared_ptr<ID3D11ShaderResourceView> &Texture::Impl::getTextureView() const {
	return textureView;
}

Texture::Impl Texture::Empty() {
	return {
		Texture{
			.width = 1,
			.height = 1,
			.channels = 4,
			.data = emptyData,
			.dynamic = false,
		},
	};
}