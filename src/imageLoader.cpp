#include "imageLoader.hpp"

#include "sail-c++/image_input.h"
#include "stdexcept"
#include "cstring"

using namespace squi;

ImageLoadRes squi::loadImageInto(unsigned char *data, size_t length) {
	sail::image_input img_input{data, length};
	auto img = img_input.next_frame();

	if (!img.is_valid()) {
		throw std::runtime_error("Failed to load image");
	}

	auto res = img.convert_to(SailPixelFormat::SAIL_PIXEL_FORMAT_BPP32_RGBA);

	if (!img.is_valid()) {
		throw std::runtime_error("Failed to convert image");
	}

	ImageLoadRes ret{
		.width = res.width(),
		.height = res.height(),
		.channels = 4,
	};


	ret.data.resize(static_cast<size_t>(ret.width) * ret.height * ret.channels);
	std::memcpy(ret.data.data(), res.pixels(), static_cast<size_t>(ret.width) * ret.height * ret.channels);

    return ret;
}
