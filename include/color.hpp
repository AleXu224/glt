#pragma once
#include "string_view"
#include "glm/glm.hpp"
#include <cstdint>

namespace squi {
	struct Color {
		uint8_t r;
		uint8_t g;
		uint8_t b;
		uint8_t a;

		Color() = default;
		Color(uint32_t value);
		Color(std::string_view hex);
		Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);
		Color(float r, float g, float b, float a = 1.0f);
		Color(const glm::vec4 &vec);

		[[nodiscard]] operator glm::vec4() const;

		[[nodiscard]] Color transistion(const Color &other, float t) const;

		[[nodiscard]] Color mix(const Color &other) const;

		[[nodiscard]] bool operator==(const Color &other) const;

		[[nodiscard]] Color operator*(const float &multiplier) const;
	};
}// namespace squi
