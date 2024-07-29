#pragma once

#include "vector"
#include "cstdint"

namespace squi {
	struct ImageLoadRes {
		uint32_t width;
		uint32_t height;
		uint32_t channels;
		std::vector<uint8_t> data;
	};

	ImageLoadRes loadImageInto(unsigned char *data, size_t length);
}// namespace squi
