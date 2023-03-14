#include "color.hpp"

using namespace squi;

Color Color::HEX(uint32_t value) {
	return {value};
}

Color Color::HEX(std::string_view hex) {
	if (hex.starts_with('#')) hex.remove_prefix(1);
	if (hex.size() != 6 && hex.size() != 8) return {0};
	uint32_t value = 0;
	for (auto c: hex) {
		if (c >= '0' && c <= '9') {
			value |= c - '0';
		} else if (c >= 'a' && c <= 'f') {
			value |= c - 'a' + 10;
		} else if (c >= 'A' && c <= 'F') {
			value |= c - 'A' + 10;
		} else {
			return {0};
		}
		value <<= 4;
	}
	if (hex.size() == 6) {
		value <<= 4;
		value |= 0xFF;
	}
	return {value};
}

Color Color::RGBA255(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
	return {
		static_cast<uint32_t>(r) << 24 |
		static_cast<uint32_t>(g) << 16 |
		static_cast<uint32_t>(b) << 8 |
		static_cast<uint32_t>(a) << 0};
}

Color Color::RGBA(float r, float g, float b, float a) {
	return RGBA255(
		static_cast<uint8_t>(r * 255),
		static_cast<uint8_t>(g * 255),
		static_cast<uint8_t>(b * 255),
		static_cast<uint8_t>(a * 255));
}

Color Color::VEC4(const glm::vec4 &vec) {
	return RGBA(vec.r, vec.g, vec.b, vec.a);
}

uint8_t Color::r() const {
	return (value >> 24) & 0xFF;
}

uint8_t Color::g() const {
	return (value >> 16) & 0xFF;
}

uint8_t Color::b() const {
	return (value >> 8) & 0xFF;
}

uint8_t Color::a() const {
	return (value >> 0) & 0xFF;
}

Color::operator glm::vec4() const {
	return {
		static_cast<float>(r()) / 255.0f,
		static_cast<float>(g()) / 255.0f,
		static_cast<float>(b()) / 255.0f,
		static_cast<float>(a()) / 255.0f};
}

// TODO: Add support for transition curves
Color Color::transistion(const Color &other, float t) const {
	glm::vec4 color1 = *this;
	glm::vec4 color2 = other;

	// Convert to premultiplied alpha
	color1.r *= color1.a;
	color1.g *= color1.a;
	color1.b *= color1.a;
	color2.r *= color2.a;
	color2.g *= color2.a;
	color2.b *= color2.a;

	// Interpolate
	auto r = color1.r * (1.0f - t) + color2.r * t;
	auto g = color1.g * (1.0f - t) + color2.g * t;
	auto b = color1.b * (1.0f - t) + color2.b * t;
	auto a = color1.a * (1.0f - t) + color2.a * t;

	// Convert back to straight alpha
	r /= a;
	g /= a;
	b /= a;

	return RGBA(r, g, b, a);
}