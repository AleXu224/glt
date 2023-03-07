#include "vec2.hpp"

using namespace squi;

vec2 vec2::operator+(const vec2 &other) const {
    return vec2(x + other.x, y + other.y);
}

vec2 vec2::operator-(const vec2 &other) const {
    return vec2(x - other.x, y - other.y);
}

vec2 vec2::operator*(const vec2 &other) const {
    return vec2(x * other.x, y * other.y);
}

vec2 vec2::operator/(const vec2 &other) const {
    return vec2(x / other.x, y / other.y);
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

vec2 vec2::withX(const float &newX) const {
    return vec2(newX, y);
}

vec2 vec2::withY(const float &newY) const {
    return vec2(x, newY);
}

vec2 vec2::withXOffset(const float &offset) const {
    return vec2(x + offset, y);
}

vec2 vec2::withYOffset(const float &offset) const {
    return vec2(x, y + offset);
}