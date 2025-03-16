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
Color::Color(int r, int g, int b, int a) : r(r), g(g), b(b), a(a) {}

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

Color Color::css(int r, int g, int b, float a) {
	return {r, g, b, static_cast<int>(a * 255.f)};
}

Color Color::css(int rgb, float a) {
	return {static_cast<uint32_t>((rgb << 8) + static_cast<uint8_t>(a * 255.f))};
}

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
	return {r, g, b, std::clamp(static_cast<uint8_t>(static_cast<float>(a) * multiplier), (uint8_t) 0, (uint8_t) 255)};
}

Color Color::transparent = Color::css(0x0, 0.f);
Color Color::black = Color::css(0x0, 1.f);

Color Color::khaki = Color::css(0xf0e68c);
Color Color::lavender = Color::css(0xe6e6fa);
Color Color::lavenderblush = Color::css(0xfff0f5);
Color Color::lawngreen = Color::css(0x7cfc00);
Color Color::lemonchiffon = Color::css(0xfffacd);
Color Color::lightblue = Color::css(0xadd8e6);
Color Color::lightcoral = Color::css(0xf08080);
Color Color::lightcyan = Color::css(0xe0ffff);
Color Color::lightgoldenrodyellow = Color::css(0xfafad2);
Color Color::lightgray = Color::css(0xd3d3d3);
Color Color::lightgreen = Color::css(0x90ee90);
Color Color::lightgrey = Color::css(0xd3d3d3);
Color Color::lightpink = Color::css(0xffb6c1);
Color Color::lightsalmon = Color::css(0xffa07a);
Color Color::lightseagreen = Color::css(0x20b2aa);
Color Color::lightskyblue = Color::css(0x87cefa);
Color Color::lightslategray = Color::css(0x778899);
Color Color::lightslategrey = Color::css(0x778899);
Color Color::lightsteelblue = Color::css(0xb0c4de);
Color Color::lightyellow = Color::css(0xffffe0);
Color Color::lime = Color::css(0x00ff00);
Color Color::limegreen = Color::css(0x32cd32);
Color Color::linen = Color::css(0xfaf0e6);
Color Color::magenta = Color::css(0xff00ff);
Color Color::maroon = Color::css(0x800000);
Color Color::mediumaquamarine = Color::css(0x66cdaa);
Color Color::mediumblue = Color::css(0x0000cd);
Color Color::mediumorchid = Color::css(0xba55d3);
Color Color::mediumpurple = Color::css(0x9370db);
Color Color::mediumseagreen = Color::css(0x3cb371);
Color Color::mediumslateblue = Color::css(0x7b68ee);
Color Color::mediumspringgreen = Color::css(0x00fa9a);
Color Color::mediumturquoise = Color::css(0x48d1cc);
Color Color::mediumvioletred = Color::css(0xc71585);
Color Color::midnightblue = Color::css(0x191970);
Color Color::mintcream = Color::css(0xf5fffa);
Color Color::mistyrose = Color::css(0xffe4e1);
Color Color::moccasin = Color::css(0xffe4b5);
Color Color::navajowhite = Color::css(0xffdead);
Color Color::navy = Color::css(0x000080);
Color Color::oldlace = Color::css(0xfdf5e6);
Color Color::olive = Color::css(0x808000);
Color Color::olivedrab = Color::css(0x6b8e23);
Color Color::orange = Color::css(0xffa500);
Color Color::orangered = Color::css(0xff4500);
Color Color::orchid = Color::css(0xda70d6);
Color Color::palegoldenrod = Color::css(0xeee8aa);
Color Color::palegreen = Color::css(0x98fb98);
Color Color::paleturquoise = Color::css(0xafeeee);
Color Color::palevioletred = Color::css(0xdb7093);
Color Color::papayawhip = Color::css(0xffefd5);
Color Color::peachpuff = Color::css(0xffdab9);
Color Color::peru = Color::css(0xcd853f);
Color Color::pink = Color::css(0xffc0cb);
Color Color::plum = Color::css(0xdda0dd);
Color Color::powderblue = Color::css(0xb0e0e6);
Color Color::purple = Color::css(0x800080);
Color Color::red = Color::css(0xff0000);
Color Color::rosybrown = Color::css(0xbc8f8f);
Color Color::royalblue = Color::css(0x4169e1);
Color Color::saddlebrown = Color::css(0x8b4513);
Color Color::salmon = Color::css(0xfa8072);
Color Color::sandybrown = Color::css(0xf4a460);
Color Color::seagreen = Color::css(0x2e8b57);
Color Color::seashell = Color::css(0xfff5ee);
Color Color::sienna = Color::css(0xa0522d);
Color Color::silver = Color::css(0xc0c0c0);
Color Color::skyblue = Color::css(0x87ceeb);
Color Color::slateblue = Color::css(0x6a5acd);
Color Color::slategray = Color::css(0x708090);
Color Color::slategrey = Color::css(0x708090);
Color Color::snow = Color::css(0xfffafa);
Color Color::springgreen = Color::css(0x00ff7f);
Color Color::steelblue = Color::css(0x4682b4);
Color Color::tan = Color::css(0xd2b48c);
Color Color::teal = Color::css(0x008080);
Color Color::thistle = Color::css(0xd8bfd8);
Color Color::tomato = Color::css(0xff6347);
Color Color::turquoise = Color::css(0x40e0d0);
Color Color::violet = Color::css(0xee82ee);
Color Color::wheat = Color::css(0xf5deb3);
Color Color::white = Color::css(0xffffff);
Color Color::whitesmoke = Color::css(0xf5f5f5);
Color Color::yellow = Color::css(0xffff00);
Color Color::yellowgreen = Color::css(0x9acd32);