#pragma once

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
    return {std::max(a.x, b.x), std::max(a.y, b.y)};
}

template <typename T>
sf::Vector2<T> min(const sf::Vector2<T> &a, const sf::Vector2<T> &b) {
    return {std::min(a.x, b.x), std::min(a.y, b.y)};
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
sf::Vector2<T> operator/(const sf::Vector2<T> &a, const sf::Vector2<T> &b) {
    return {a.x / b.x, a.y / b.y};
}

template <typename T, typename U>
sf::Vector2<T> operator/(const sf::Vector2<T> &a, U b) {
    return {a.x / b, a.y / b};
}

template <typename T, typename U>
sf::Vector2<T> operator/(U a, const sf::Vector2<T> &b) {
    return {a / b.x, a / b.y};
}

template <typename T>
sf::Vector2<T> operator*(const sf::Vector2<T> &a, const sf::Vector2<T> &b) {
    return {a.x * b.x, a.y * b.y};
}

template <typename T, typename U>
sf::Vector2<T> operator*(const sf::Vector2<T> &a, U b) {
    return {a.x * b, a.y * b};
}

template <typename T, typename U>
sf::Vector2<T> operator*(U a, const sf::Vector2<T> &b) {
    return {a * b.x, a * b.y};
}

template <typename T, typename U>
sf::Vector2<T> operator+(const sf::Vector2<T> &a, U b) {
    return {a.x + b, a.y + b};
}

template <typename T, typename U>
sf::Vector2<T> operator+(U a, const sf::Vector2<T> &b) {
    return {a + b.x, a + b.y};
}

template <typename T, typename U>
sf::Vector2<T> operator-(const sf::Vector2<T> &a, U b) {
    return {a.x - b, a.y - b};
}

template <typename T, typename U>
sf::Vector2<T> operator-(U a, const sf::Vector2<T> &b) {
    return {a - b.x, a - b.y};
}

namespace std {
    template <typename T>
    std::ostream &operator<<(std::ostream &stream, const sf::Vector2<T> &a) {
        stream << "Vector(" << a.x << ", " << a.y << ")";
        return stream;
    }
}  // namespace std

#endif  // VECTOR_OPERATIONS_H
