#include "loader.hpp"

#include "cstring"
#include "sail-c++/image_input.h"
#include "sail-common/log.h"
#include "stdexcept"


using namespace squi;

ImageLoadRes squi::loadImageInto(unsigned char *data, size_t length) {
	[[maybe_unused]] static bool disableSailLogging = []() {
		sail_set_log_barrier(SailLogLevel::SAIL_LOG_LEVEL_SILENCE);
		return true;
	}();

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
