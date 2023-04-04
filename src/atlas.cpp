#include "atlas.hpp"
#include "renderer.hpp"

using namespace squi;

Atlas::Atlas() {
	ID3D11Texture2D *tex;
	D3D11_TEXTURE2D_DESC desc{};
	desc.Width = ATLAS_SIZE;
	desc.Height = ATLAS_SIZE;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	D3D11_SUBRESOURCE_DATA data{};
	data.pSysMem = atlas->data();
	data.SysMemPitch = ATLAS_SIZE;

	auto device = Renderer::getInstance().getDevice();
	device->CreateTexture2D(&desc, &data, &tex);
	texture = std::shared_ptr<ID3D11Texture2D>(tex, [](ID3D11Texture2D *tex) { tex->Release(); });

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_R8_UNORM;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	ID3D11ShaderResourceView *srv;
	device->CreateShaderResourceView(texture.get(), &srvDesc, &srv);
	textureView = std::shared_ptr<ID3D11ShaderResourceView>(srv, [](ID3D11ShaderResourceView *srv) { srv->Release(); });
}

std::tuple<vec2, vec2, bool> Atlas::add(const uint16_t& width, const uint16_t& height, unsigned char *data) {
	AtlasRow *usedRow = nullptr;
	for (auto &row: rows) {
		if (row.availableWidth >= width) {
			if (row.height < height) {
				if (row.canBeMadeTaller && availableHeight >= height - row.height) {
					row.height = height;
					availableHeight -= height - row.height;
				} else {
					continue;
				}
			}
		} else {
			continue;
		}

		usedRow = &row;

	}
	if (usedRow == nullptr) {
		if (availableHeight < height) return {vec2(0, 0), vec2(0, 0), false};
		// Set last row to be not able to be made taller
		if (!rows.empty()) rows.back().canBeMadeTaller = false;

		AtlasRow newRow{
			.yOffset = static_cast<uint16_t>(ATLAS_SIZE - availableHeight),
			.height = static_cast<uint16_t>(height),
			.availableWidth = static_cast<uint16_t>(ATLAS_SIZE),
			.canBeMadeTaller = true
		};
		rows.emplace_back(std::move(newRow));

		usedRow = &rows.back();
		availableHeight -= height;
	}

	usedRow->elements.emplace_back(AtlasElement{
		.xOffset = static_cast<uint16_t>(ATLAS_SIZE - usedRow->availableWidth),
		.width = static_cast<uint16_t>(width),
		.height = static_cast<uint16_t>(height)
	});

	// Copy data to atlas
	for (int y = 0; y < height; y++) {
		memcpy(&atlas->at((y + usedRow->yOffset) * ATLAS_SIZE + ATLAS_SIZE - usedRow->availableWidth), data + y * width, width);
	}

	auto context = Renderer::getInstance().getDeviceContext();
//	D3D11_BOX region = {
//		.left = static_cast<UINT>(ATLAS_SIZE - usedRow->availableWidth),
//		.top = static_cast<UINT>(usedRow->yOffset),
//		.front = 0,
//		.right = static_cast<UINT>(ATLAS_SIZE - usedRow->availableWidth + width),
//		.bottom = static_cast<UINT>(usedRow->yOffset + height),
//		.back = 1,
//	};
//	context->UpdateSubresource(texture.get(), 0, &region, data, width, 0);
	// The above code should be better but doesn't seem to work on older OSes
	textureDirty = true;

	// Prepare return values
	vec2 uvTopLeft{
		static_cast<float>(ATLAS_SIZE - usedRow->availableWidth) / static_cast<float>(ATLAS_SIZE),
		static_cast<float>(usedRow->yOffset) / static_cast<float>(ATLAS_SIZE)
	};
	vec2 uvBottomRight{
		static_cast<float>(ATLAS_SIZE - usedRow->availableWidth + width) / static_cast<float>(ATLAS_SIZE),
		static_cast<float>(usedRow->yOffset + height) / static_cast<float>(ATLAS_SIZE)
	};

	usedRow->availableWidth -= width;

	return {uvTopLeft, uvBottomRight, true};
}

void Atlas::updateTexture() {
	if (!textureDirty) return;
	auto context = Renderer::getInstance().getDeviceContext();
//	context->UpdateSubresource(texture.get(), 0, nullptr, atlas->data(), ATLAS_SIZE, 0);
	// Again, the above should be better but doesn't work on older OSes
	D3D11_MAPPED_SUBRESOURCE mapped{};
	context->Map(texture.get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
	memcpy((unsigned char *)mapped.pData, atlas->data(), ATLAS_SIZE * ATLAS_SIZE);
	context->Unmap(texture.get(), 0);
}