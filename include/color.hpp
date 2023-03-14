#ifndef SQUI_COLOR_HPP
#define SQUI_COLOR_HPP

#include "glm/glm.hpp"
#include "stdint.h"
#include "string_view"

namespace squi {
	struct Color {
		uint32_t value;

		static Color HEX(uint32_t value);
		static Color HEX(std::string_view hex);
		static Color RGBA255(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);
		static Color RGBA(float r, float g, float b, float a = 1.0f);
		static Color VEC4(const glm::vec4 &vec);

		[[nodiscard]] uint8_t r() const;
		[[nodiscard]] uint8_t g() const;
		[[nodiscard]] uint8_t b() const;
		[[nodiscard]] uint8_t a() const;

		[[nodiscard]] operator glm::vec4() const;

		[[nodiscard]] Color transistion(const Color &other, float t) const;
	};
}// namespace squi

#endif//SQUI_COLOR_HPP
