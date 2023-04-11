#include "vec2.hpp"

using namespace squi;

vec2 vec2::operator+(const vec2 &other) const {
    return {x + other.x, y + other.y};
}

vec2 vec2::operator-(const vec2 &other) const {
    return {x - other.x, y - other.y};
}

vec2 vec2::operator*(const vec2 &other) const {
    return {x * other.x, y * other.y};
}

vec2 vec2::operator/(const vec2 &other) const {
    return {x / other.x, y / other.y};
}

void vec2::operator+=(const vec2 &other) {
    x += other.x;
    y += other.y;
}

void vec2::operator-=(const vec2 &other) {
    x -= other.x;
    y -= other.y;
}

void vec2::operator*=(const vec2 &other) {
    x *= other.x;
    y *= other.y;
}

void vec2::operator/=(const vec2 &other) {
    x /= other.x;
    y /= other.y;
}

bool vec2::operator==(const vec2 &other) const {
    return x == other.x && y == other.y;
}

vec2 vec2::withX(const float &newX) const {
    return {newX, y};
}

vec2 vec2::withY(const float &newY) const {
    return {x, newY};
}

vec2 vec2::withXOffset(const float &offset) const {
    return {x + offset, y};
}

vec2 vec2::withYOffset(const float &offset) const {
    return {x, y + offset};
}

vec2 vec2::rounded() const {
    return {std::round(x), std::round(y)};
}

vec2::operator DirectX::XMFLOAT2() const {
	return {x, y};
}
