#include "color.hpp"
#include "algorithm"
#include <glm/fwd.hpp>

using namespace squi;

Color::Color(uint32_t value)
	: r(value >> 24),
	  g((value >> 16) & 0xFF),
	  b((value >> 8) & 0xFF),
	  a((value) & 0xFF) {}

Color::Color(std::string_view hex) : r(), g(), b(), a() {
	if (hex.starts_with('#')) hex.remove_prefix(1);
	if (hex.size() != 6 && hex.size() != 8) {
		r = 0;
		g = 0;
		b = 0;
		a = 0;
	}
	uint32_t value = 0;
	for (auto c: hex) {
		if (c >= '0' && c <= '9') {
			value |= c - '0';
		} else if (c >= 'a' && c <= 'f') {
			value |= c - 'a' + 10;
		} else if (c >= 'A' && c <= 'F') {
			value |= c - 'A' + 10;
		} else {
			r = 0;
			g = 0;
			b = 0;
			a = 0;
		}
		value <<= 4;
	}
	if (hex.size() == 6) {
		value <<= 4;
		value |= 0xFF;
	}

	*this = Color(value);
}

Color::Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a) : r(r), g(g), b(b), a(a) {}

Color::Color(float r, float g, float b, float a)
	: r(static_cast<uint8_t>(r * 255.f)),
	  g(static_cast<uint8_t>(g * 255.f)),
	  b(static_cast<uint8_t>(b * 255.f)),
	  a(static_cast<uint8_t>(a * 255.f)) {}

Color::Color(const glm::vec4 &vec)
	: r(static_cast<uint8_t>(vec.r * 255.f)),
	  g(static_cast<uint8_t>(vec.g * 255.f)),
	  b(static_cast<uint8_t>(vec.b * 255.f)),
	  a(static_cast<uint8_t>(vec.a * 255.f)) {}

Color::operator glm::vec4() const {
	return {
		static_cast<float>(r) / 255.0f,
		static_cast<float>(g) / 255.0f,
		static_cast<float>(b) / 255.0f,
		static_cast<float>(a) / 255.0f,
	};
}

// TODO: Add support for transition curves
Color Color::transistion(const Color &other, float t) const {
	glm::vec4 color1 = *this;
	glm::vec4 color2 = other;

	// Convert to premultiplied alpha
	color1.x *= color1.w;
	color1.y *= color1.w;
	color1.z *= color1.w;
	color2.x *= color2.w;
	color2.y *= color2.w;
	color2.z *= color2.w;

	// Interpolate
	auto newR = color1.x * (1.0f - t) + color2.x * t;
	auto newG = color1.y * (1.0f - t) + color2.y * t;
	auto newB = color1.z * (1.0f - t) + color2.z * t;
	auto newA = color1.w * (1.0f - t) + color2.w * t;

	// Convert back to straight alpha
	newR /= newA;
	newG /= newA;
	newB /= newA;

	return {newR, newG, newB, newA};
}

Color Color::mix(const Color &other) const {
	glm::vec4 color1 = *this;
	glm::vec4 color2 = other;

	const float newA = (1 - color1.w) * color2.w + color1.w;
	const float newR = ((1 - color1.w) * color2.w * color2.x + color1.w * color1.x) / newA;
	const float newG = ((1 - color1.w) * color2.w * color2.y + color1.w * color1.y) / newA;
	const float newB = ((1 - color1.w) * color2.w * color2.z + color1.w * color1.z) / newA;
	return {newR, newG, newB, newA};
}

bool Color::operator==(const Color &other) const {
	return r == other.r && g == other.g && b == other.b && a == other.a;
}

Color Color::operator*(const float &multiplier) const {
	return {r, g, b,
			std::clamp(static_cast<uint8_t>(static_cast<float>(a) * multiplier),
					   (uint8_t) 0, (uint8_t) 255)};
}