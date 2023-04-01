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

Color Color::VEC4(const DirectX::XMFLOAT4 &vec) {
	return RGBA(vec.z, vec.y, vec.z, vec.w);
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

Color::operator DirectX::XMFLOAT4() const {
	return {
		static_cast<float>(r()) / 255.0f,
		static_cast<float>(g()) / 255.0f,
		static_cast<float>(b()) / 255.0f,
		static_cast<float>(a()) / 255.0f};
}

// TODO: Add support for transition curves
Color Color::transistion(const Color &other, float t) const {
	DirectX::XMFLOAT4 color1 = *this;
	DirectX::XMFLOAT4 color2 = other;

	// Convert to premultiplied alpha
	color1.x *= color1.w;
	color1.y *= color1.w;
	color1.z *= color1.w;
	color2.x *= color2.w;
	color2.y *= color2.w;
	color2.z *= color2.w;

	// Interpolate
	auto r = color1.x * (1.0f - t) + color2.x * t;
	auto g = color1.y * (1.0f - t) + color2.y * t;
	auto b = color1.z * (1.0f - t) + color2.z * t;
	auto a = color1.w * (1.0f - t) + color2.w * t;

	// Convert back to straight alpha
	r /= a;
	g /= a;
	b /= a;

	return RGBA(r, g, b, a);
}