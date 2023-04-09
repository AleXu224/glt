#include "texture.hpp"
#include "stdexcept"
#include "renderer.hpp"

using namespace squi;

Texture::Impl::Impl(const Texture &args) {
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
            format = DXGI_FORMAT_R8G8B8A8_UNORM;
            break;
        case 4:
            format = DXGI_FORMAT_R8G8B8A8_UNORM;
            break;
        default:
            throw std::runtime_error("Invalid number of channels");
    }
    textureDesc.Format = format;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.Usage = args.dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    textureDesc.CPUAccessFlags = args.dynamic ? D3D11_CPU_ACCESS_WRITE : 0;
    textureDesc.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA subresourceData{};
    subresourceData.pSysMem = args.data.get();
    subresourceData.SysMemPitch = args.width * args.channels;
    subresourceData.SysMemSlicePitch = 0;

    auto device = Renderer::getInstance().getDevice();
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

const std::shared_ptr<ID3D11ShaderResourceView> &Texture::Impl::getTextureView() const {
    return textureView;
}