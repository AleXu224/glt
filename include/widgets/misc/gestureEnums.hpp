#pragma once

#include <cstdint>

namespace squi {
	struct GestureMod {
		static constexpr int8_t none = 0x0000;
		static constexpr int8_t shift = 0x0001;
		static constexpr int8_t control = 0x0002;
		static constexpr int8_t alt = 0x0004;
		static constexpr int8_t super = 0x0008;
		static constexpr int8_t capsLock = 0x0010;
		static constexpr int8_t numLock = 0x002;

		int8_t value = none;

		constexpr GestureMod() = default;
		constexpr GestureMod(int8_t v) : value(v) {}

		constexpr auto operator<=>(const GestureMod &) const = default;
	};

	enum class GestureAction : int8_t {
		release = 0,
		press = 1,
		repeat = 2
	};

	struct GestureMouseKey {
		int8_t value;
		constexpr GestureMouseKey(int8_t v) : value(v) {}

		GestureMouseKey operator+(int8_t v) const {
			return GestureMouseKey{static_cast<int8_t>(value + v)};
		}

		constexpr auto operator<=>(const GestureMouseKey &) const = default;


		static const GestureMouseKey button1;
		static const GestureMouseKey button2;
		static const GestureMouseKey button3;
		static const GestureMouseKey button4;
		static const GestureMouseKey button5;
		static const GestureMouseKey button6;
		static const GestureMouseKey button7;
		static const GestureMouseKey button8;
		static const GestureMouseKey left;
		static const GestureMouseKey right;
		static const GestureMouseKey middle;
	};
	inline constexpr GestureMouseKey GestureMouseKey::button1 = 0;
	inline constexpr GestureMouseKey GestureMouseKey::button2 = 1;
	inline constexpr GestureMouseKey GestureMouseKey::button3 = 2;
	inline constexpr GestureMouseKey GestureMouseKey::button4 = 3;
	inline constexpr GestureMouseKey GestureMouseKey::button5 = 4;
	inline constexpr GestureMouseKey GestureMouseKey::button6 = 5;
	inline constexpr GestureMouseKey GestureMouseKey::button7 = 6;
	inline constexpr GestureMouseKey GestureMouseKey::button8 = 7;
	inline constexpr GestureMouseKey GestureMouseKey::left = button1;
	inline constexpr GestureMouseKey GestureMouseKey::right = button2;
	inline constexpr GestureMouseKey GestureMouseKey::middle = button3;

	struct GestureKey {
		int16_t value;

		constexpr GestureKey(int16_t v) : value(v) {}

		GestureKey operator+(int16_t v) const {
			return GestureKey{static_cast<int16_t>(value + v)};
		}

		constexpr auto operator<=>(const GestureKey &) const = default;

		static const GestureKey space;
		static const GestureKey apostrophe;
		static const GestureKey comma;
		static const GestureKey minus;
		static const GestureKey period;
		static const GestureKey slash;
		static const GestureKey n0;
		static const GestureKey n1;
		static const GestureKey n2;
		static const GestureKey n3;
		static const GestureKey n4;
		static const GestureKey n5;
		static const GestureKey n6;
		static const GestureKey n7;
		static const GestureKey n8;
		static const GestureKey n9;
		static const GestureKey semicolon;
		static const GestureKey equal;
		static const GestureKey a;
		static const GestureKey b;
		static const GestureKey c;
		static const GestureKey d;
		static const GestureKey e;
		static const GestureKey f;
		static const GestureKey g;
		static const GestureKey h;
		static const GestureKey i;
		static const GestureKey j;
		static const GestureKey k;
		static const GestureKey l;
		static const GestureKey m;
		static const GestureKey n;
		static const GestureKey o;
		static const GestureKey p;
		static const GestureKey q;
		static const GestureKey r;
		static const GestureKey s;
		static const GestureKey t;
		static const GestureKey u;
		static const GestureKey v;
		static const GestureKey w;
		static const GestureKey x;
		static const GestureKey y;
		static const GestureKey z;
		static const GestureKey leftBracket;
		static const GestureKey backslash;
		static const GestureKey rightBracket;
		static const GestureKey graveAccent;
		static const GestureKey world1;
		static const GestureKey world2;

		/* Function keys */
		static const GestureKey escape;
		static const GestureKey enter;
		static const GestureKey tab;
		static const GestureKey backspace;
		static const GestureKey insert;
		static const GestureKey del;
		static const GestureKey right;
		static const GestureKey left;
		static const GestureKey down;
		static const GestureKey up;
		static const GestureKey pageUp;
		static const GestureKey pageDown;
		static const GestureKey home;
		static const GestureKey end;
		static const GestureKey capsLock;
		static const GestureKey scrollLock;
		static const GestureKey numLock;
		static const GestureKey printScreen;
		static const GestureKey pause;
		static const GestureKey f1;
		static const GestureKey f2;
		static const GestureKey f3;
		static const GestureKey f4;
		static const GestureKey f5;
		static const GestureKey f6;
		static const GestureKey f7;
		static const GestureKey f8;
		static const GestureKey f9;
		static const GestureKey f10;
		static const GestureKey f11;
		static const GestureKey f12;
		static const GestureKey f13;
		static const GestureKey f14;
		static const GestureKey f15;
		static const GestureKey f16;
		static const GestureKey f17;
		static const GestureKey f18;
		static const GestureKey f19;
		static const GestureKey f20;
		static const GestureKey f21;
		static const GestureKey f22;
		static const GestureKey f23;
		static const GestureKey f24;
		static const GestureKey f25;
		static const GestureKey kp_0;
		static const GestureKey kp_1;
		static const GestureKey kp_2;
		static const GestureKey kp_3;
		static const GestureKey kp_4;
		static const GestureKey kp_5;
		static const GestureKey kp_6;
		static const GestureKey kp_7;
		static const GestureKey kp_8;
		static const GestureKey kp_9;
		static const GestureKey kp_decimal;
		static const GestureKey kp_divide;
		static const GestureKey kp_multiply;
		static const GestureKey kp_subtract;
		static const GestureKey kp_add;
		static const GestureKey kp_enter;
		static const GestureKey kp_equal;
		static const GestureKey leftShift;
		static const GestureKey leftControl;
		static const GestureKey leftAlt;
		static const GestureKey leftSuper;
		static const GestureKey rightShift;
		static const GestureKey rightControl;
		static const GestureKey rightAlt;
		static const GestureKey rightSuper;
		static const GestureKey menu;
	};
	inline constexpr GestureKey GestureKey::space = 32;
	inline constexpr GestureKey GestureKey::apostrophe = 39 /* ' */;
	inline constexpr GestureKey GestureKey::comma = 44 /* , */;
	inline constexpr GestureKey GestureKey::minus = 45 /* - */;
	inline constexpr GestureKey GestureKey::period = 46 /* . */;
	inline constexpr GestureKey GestureKey::slash = 47 /* / */;
	inline constexpr GestureKey GestureKey::n0 = 48;
	inline constexpr GestureKey GestureKey::n1 = 49;
	inline constexpr GestureKey GestureKey::n2 = 50;
	inline constexpr GestureKey GestureKey::n3 = 51;
	inline constexpr GestureKey GestureKey::n4 = 52;
	inline constexpr GestureKey GestureKey::n5 = 53;
	inline constexpr GestureKey GestureKey::n6 = 54;
	inline constexpr GestureKey GestureKey::n7 = 55;
	inline constexpr GestureKey GestureKey::n8 = 56;
	inline constexpr GestureKey GestureKey::n9 = 57;
	inline constexpr GestureKey GestureKey::semicolon = 59 /* ; */;
	inline constexpr GestureKey GestureKey::equal = 61 /* = */;
	inline constexpr GestureKey GestureKey::a = 65;
	inline constexpr GestureKey GestureKey::b = 66;
	inline constexpr GestureKey GestureKey::c = 67;
	inline constexpr GestureKey GestureKey::d = 68;
	inline constexpr GestureKey GestureKey::e = 69;
	inline constexpr GestureKey GestureKey::f = 70;
	inline constexpr GestureKey GestureKey::g = 71;
	inline constexpr GestureKey GestureKey::h = 72;
	inline constexpr GestureKey GestureKey::i = 73;
	inline constexpr GestureKey GestureKey::j = 74;
	inline constexpr GestureKey GestureKey::k = 75;
	inline constexpr GestureKey GestureKey::l = 76;
	inline constexpr GestureKey GestureKey::m = 77;
	inline constexpr GestureKey GestureKey::n = 78;
	inline constexpr GestureKey GestureKey::o = 79;
	inline constexpr GestureKey GestureKey::p = 80;
	inline constexpr GestureKey GestureKey::q = 81;
	inline constexpr GestureKey GestureKey::r = 82;
	inline constexpr GestureKey GestureKey::s = 83;
	inline constexpr GestureKey GestureKey::t = 84;
	inline constexpr GestureKey GestureKey::u = 85;
	inline constexpr GestureKey GestureKey::v = 86;
	inline constexpr GestureKey GestureKey::w = 87;
	inline constexpr GestureKey GestureKey::x = 88;
	inline constexpr GestureKey GestureKey::y = 89;
	inline constexpr GestureKey GestureKey::z = 90;
	inline constexpr GestureKey GestureKey::leftBracket = 91 /* [ */;
	inline constexpr GestureKey GestureKey::backslash = 92 /* \ */;
	inline constexpr GestureKey GestureKey::rightBracket = 93 /* ] */;
	inline constexpr GestureKey GestureKey::graveAccent = 96 /* ` */;
	inline constexpr GestureKey GestureKey::world1 = 161 /* non-US #1 */;
	inline constexpr GestureKey GestureKey::world2 = 162 /* non-US #2 */;

	/* Function keys */
	inline constexpr GestureKey GestureKey::escape = 256;
	inline constexpr GestureKey GestureKey::enter = 257;
	inline constexpr GestureKey GestureKey::tab = 258;
	inline constexpr GestureKey GestureKey::backspace = 259;
	inline constexpr GestureKey GestureKey::insert = 260;
	inline constexpr GestureKey GestureKey::del = 261;
	inline constexpr GestureKey GestureKey::right = 262;
	inline constexpr GestureKey GestureKey::left = 263;
	inline constexpr GestureKey GestureKey::down = 264;
	inline constexpr GestureKey GestureKey::up = 265;
	inline constexpr GestureKey GestureKey::pageUp = 266;
	inline constexpr GestureKey GestureKey::pageDown = 267;
	inline constexpr GestureKey GestureKey::home = 268;
	inline constexpr GestureKey GestureKey::end = 269;
	inline constexpr GestureKey GestureKey::capsLock = 280;
	inline constexpr GestureKey GestureKey::scrollLock = 281;
	inline constexpr GestureKey GestureKey::numLock = 282;
	inline constexpr GestureKey GestureKey::printScreen = 283;
	inline constexpr GestureKey GestureKey::pause = 284;
	inline constexpr GestureKey GestureKey::f1 = 290;
	inline constexpr GestureKey GestureKey::f2 = 291;
	inline constexpr GestureKey GestureKey::f3 = 292;
	inline constexpr GestureKey GestureKey::f4 = 293;
	inline constexpr GestureKey GestureKey::f5 = 294;
	inline constexpr GestureKey GestureKey::f6 = 295;
	inline constexpr GestureKey GestureKey::f7 = 296;
	inline constexpr GestureKey GestureKey::f8 = 297;
	inline constexpr GestureKey GestureKey::f9 = 298;
	inline constexpr GestureKey GestureKey::f10 = 299;
	inline constexpr GestureKey GestureKey::f11 = 300;
	inline constexpr GestureKey GestureKey::f12 = 301;
	inline constexpr GestureKey GestureKey::f13 = 302;
	inline constexpr GestureKey GestureKey::f14 = 303;
	inline constexpr GestureKey GestureKey::f15 = 304;
	inline constexpr GestureKey GestureKey::f16 = 305;
	inline constexpr GestureKey GestureKey::f17 = 306;
	inline constexpr GestureKey GestureKey::f18 = 307;
	inline constexpr GestureKey GestureKey::f19 = 308;
	inline constexpr GestureKey GestureKey::f20 = 309;
	inline constexpr GestureKey GestureKey::f21 = 310;
	inline constexpr GestureKey GestureKey::f22 = 311;
	inline constexpr GestureKey GestureKey::f23 = 312;
	inline constexpr GestureKey GestureKey::f24 = 313;
	inline constexpr GestureKey GestureKey::f25 = 314;
	inline constexpr GestureKey GestureKey::kp_0 = 320;
	inline constexpr GestureKey GestureKey::kp_1 = 321;
	inline constexpr GestureKey GestureKey::kp_2 = 322;
	inline constexpr GestureKey GestureKey::kp_3 = 323;
	inline constexpr GestureKey GestureKey::kp_4 = 324;
	inline constexpr GestureKey GestureKey::kp_5 = 325;
	inline constexpr GestureKey GestureKey::kp_6 = 326;
	inline constexpr GestureKey GestureKey::kp_7 = 327;
	inline constexpr GestureKey GestureKey::kp_8 = 328;
	inline constexpr GestureKey GestureKey::kp_9 = 329;
	inline constexpr GestureKey GestureKey::kp_decimal = 330;
	inline constexpr GestureKey GestureKey::kp_divide = 331;
	inline constexpr GestureKey GestureKey::kp_multiply = 332;
	inline constexpr GestureKey GestureKey::kp_subtract = 333;
	inline constexpr GestureKey GestureKey::kp_add = 334;
	inline constexpr GestureKey GestureKey::kp_enter = 335;
	inline constexpr GestureKey GestureKey::kp_equal = 336;
	inline constexpr GestureKey GestureKey::leftShift = 340;
	inline constexpr GestureKey GestureKey::leftControl = 341;
	inline constexpr GestureKey GestureKey::leftAlt = 342;
	inline constexpr GestureKey GestureKey::leftSuper = 343;
	inline constexpr GestureKey GestureKey::rightShift = 344;
	inline constexpr GestureKey GestureKey::rightControl = 345;
	inline constexpr GestureKey GestureKey::rightAlt = 346;
	inline constexpr GestureKey GestureKey::rightSuper = 347;
	inline constexpr GestureKey GestureKey::menu = 348;
}// namespace squi

// hash operator
namespace std {
	template<>
	struct hash<squi::GestureMouseKey> {
		std::size_t operator()(const squi::GestureMouseKey &k) const {
			return std::hash<int8_t>()(k.value);
		}
	};
	template<>
	struct hash<squi::GestureKey> {
		std::size_t operator()(const squi::GestureKey &k) const {
			return std::hash<int16_t>()(k.value);
		}
	};
}// namespace std