#pragma once
#include "glm/glm.hpp"// IWYU pragma: keep
#include "string_view"
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
		Color(int r, int g, int b, int a = 255);
		Color(float r, float g, float b, float a = 1.0f);
		Color(const glm::vec4 &vec);
		static Color css(int r, int g, int b, float a = 1.f);
		static Color css(int rgb, float a = 1.f);

		[[nodiscard]] operator glm::vec4() const;

		[[nodiscard]] Color transition(const Color &other, float t) const;

		[[nodiscard]] Color mix(const Color &other) const;

		[[nodiscard]] bool operator==(const Color &other) const;

		[[nodiscard]] Color operator*(const float &multiplier) const;

		[[nodiscard]] bool isLight() const;

		static Color transparent;
		static Color black;

		static Color khaki;
		static Color lavender;
		static Color lavenderblush;
		static Color lawngreen;
		static Color lemonchiffon;
		static Color lightblue;
		static Color lightcoral;
		static Color lightcyan;
		static Color lightgoldenrodyellow;
		static Color lightgray;
		static Color lightgreen;
		static Color lightgrey;
		static Color lightpink;
		static Color lightsalmon;
		static Color lightseagreen;
		static Color lightskyblue;
		static Color lightslategray;
		static Color lightslategrey;
		static Color lightsteelblue;
		static Color lightyellow;
		static Color lime;
		static Color limegreen;
		static Color linen;
		static Color magenta;
		static Color maroon;
		static Color mediumaquamarine;
		static Color mediumblue;
		static Color mediumorchid;
		static Color mediumpurple;
		static Color mediumseagreen;
		static Color mediumslateblue;
		static Color mediumspringgreen;
		static Color mediumturquoise;
		static Color mediumvioletred;
		static Color midnightblue;
		static Color mintcream;
		static Color mistyrose;
		static Color moccasin;
		static Color navajowhite;
		static Color navy;
		static Color oldlace;
		static Color olive;
		static Color olivedrab;
		static Color orange;
		static Color orangered;
		static Color orchid;
		static Color palegoldenrod;
		static Color palegreen;
		static Color paleturquoise;
		static Color palevioletred;
		static Color papayawhip;
		static Color peachpuff;
		static Color peru;
		static Color pink;
		static Color plum;
		static Color powderblue;
		static Color purple;
		static Color red;
		static Color rosybrown;
		static Color royalblue;
		static Color saddlebrown;
		static Color salmon;
		static Color sandybrown;
		static Color seagreen;
		static Color seashell;
		static Color sienna;
		static Color silver;
		static Color skyblue;
		static Color slateblue;
		static Color slategray;
		static Color slategrey;
		static Color snow;
		static Color springgreen;
		static Color steelblue;
		static Color tan;
		static Color teal;
		static Color thistle;
		static Color tomato;
		static Color turquoise;
		static Color violet;
		static Color wheat;
		static Color white;
		static Color whitesmoke;
		static Color yellow;
		static Color yellowgreen;
	};
}// namespace squi
