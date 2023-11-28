#pragma once

#include "SFML/System/Vector2.hpp"
#ifndef VECTOR_OPERATIONS_H
#define VECTOR_OPERATIONS_H

#include <SFML/System.hpp>
#include <algorithm>
#include <ostream>

template <typename T>
float dot(const sf::Vector2<T> &a, const sf::Vector2<T> &b) {
    return a.x * b.x + a.y * b.y;
}

template <typename T>
float length_squared(const sf::Vector2<T> &a) {
    return dot(a, a);
}

template <typename T>
float length(const sf::Vector2<T> &a) {
    return std::sqrt(length_squared(a));
}

template <typename T>
sf::Vector2<T> max(const sf::Vector2<T> &a, const sf::Vector2<T> &b) {
    return sf::Vector2<T>(std::max(a.x, b.x), std::max(a.y, b.y));
}

template <typename T>
sf::Vector2<T> min(const sf::Vector2<T> &a, const sf::Vector2<T> &b) {
    return sf::Vector2<T>(std::min(a.x, b.x), std::min(a.y, b.y));
}

template <typename T>
T max(const sf::Vector2<T> &a) {
    return std::max(a.x, a.y);
}

template <typename T>
T min(const sf::Vector2<T> &a) {
    return std::min(a.x, a.y);
}

template <typename T>
T mean(const sf::Vector2<T> &a) {
    return (a.x + a.y) / 2;
}

template <typename T>
sf::Vector2<T> proj(const sf::Vector2<T> &a, const sf::Vector2<T> &b) {
    return b * (dot(a, b) / dot(b, b));
}

template <typename T>
sf::Vector2<T> normalized(const sf::Vector2<T> &a) {
    T len = length(a);
    if (len) return a / len;
    return a;
}

template <typename T>
sf::Vector2<T> move_towards(const sf::Vector2<T> &current, const sf::Vector2<T> &target, T max_distance_delta) {
    sf::Vector2<T> delta = target - current;
    if (length_squared(delta) > max_distance_delta * max_distance_delta) {
        delta = normalized(delta) * max_distance_delta;
    }
    return current + delta;
}

template <typename T>
sf::Vector2<T> clamp_magnitude(const sf::Vector2<T> &a, T max_magnitude) {
    if (!max_magnitude) return sf::Vector2<T>(0, 0);
    T len = length(a);
    if (!len) return sf::Vector2<T>(0, 0);
    if (len > max_magnitude) {
        return a / len * max_magnitude;
    }
    return a;
}

template <typename T>
sf::Vector2<T> YO(const sf::Vector2<T> &a) {
    return sf::Vector2<T>(a.y, 0);
}

template <typename T>
sf::Vector2<T> OY(const sf::Vector2<T> &a) {
    return sf::Vector2<T>(0, a.y);
}

template <typename T>
sf::Vector2<T> YY(const sf::Vector2<T> &a) {
    return sf::Vector2<T>(a.y, a.y);
}

template <typename T>
sf::Vector2<T> XO(const sf::Vector2<T> &a) {
    return sf::Vector2<T>(a.x, 0);
}

template <typename T>
sf::Vector2<T> OX(const sf::Vector2<T> &a) {
    return sf::Vector2<T>(0, a.x);
}

template <typename T>
sf::Vector2<T> XX(const sf::Vector2<T> &a) {
    return sf::Vector2<T>(a.x, a.x);
}

template <typename T>
sf::Vector2<T> operator/(const sf::Vector2<T> &a, const sf::Vector2<T> &b) {
    return sf::Vector2<T>(a.x / b.x, a.y / b.y);
}

template <typename T, typename U>
sf::Vector2<T> operator/(const sf::Vector2<T> &a, U b) {
    return sf::Vector2<T>(a.x / b, a.y / b);
}

template <typename T, typename U>
sf::Vector2<T> operator/(U a, const sf::Vector2<T> &b) {
    return sf::Vector2<T>(a / b.x, a / b.y);
}

template <typename T>
sf::Vector2<T> operator*(const sf::Vector2<T> &a, const sf::Vector2<T> &b) {
    return sf::Vector2<T>(a.x * b.x, a.y * b.y);
}

template <typename T, typename U>
sf::Vector2<T> operator*(const sf::Vector2<T> &a, U b) {
    return sf::Vector2<T>(a.x * b, a.y * b);
}

template <typename T, typename U>
sf::Vector2<T> operator*(U a, const sf::Vector2<T> &b) {
    return sf::Vector2<T>(a * b.x, a * b.y);
}

template <typename T, typename U>
sf::Vector2<T> operator+(const sf::Vector2<T> &a, U b) {
    return sf::Vector2<T>(a.x + b, a.y + b);
}

template <typename T, typename U>
sf::Vector2<T> operator+(U a, const sf::Vector2<T> &b) {
    return sf::Vector2<T>(a + b.x, a + b.y);
}

template <typename T, typename U>
sf::Vector2<T> operator-(const sf::Vector2<T> &a, U b) {
    return sf::Vector2<T>(a.x - b, a.y - b);
}

template <typename T, typename U>
sf::Vector2<T> operator-(U a, const sf::Vector2<T> &b) {
    return sf::Vector2<T>(a - b.x, a - b.y);
}

namespace std {
    template <typename T>
    std::ostream &operator<<(std::ostream &stream, const sf::Vector2<T> &a) {
        stream << "Vector(" << a.x << ", " << a.y << ")";
        return stream;
    }
}  // namespace std

#endif  // VECTOR_OPERATIONS_H
