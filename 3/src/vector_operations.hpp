#pragma once

#ifndef VECTOR_OPERATIONS_H
#define VECTOR_OPERATIONS_H

#include <algorithm>

#include <SFML/System.hpp>

template <typename T>
float dot(const sf::Vector2<T> &a, const sf::Vector2<T> &b) {
    return a.x * b.x + a.y * b.y;
}

template <typename T>
float length(const sf::Vector2<T> &a) {
    return std::sqrt(dot(a, a));
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
sf::Vector2<T> proj(const sf::Vector2<T> &a, const sf::Vector2<T> &b) {
    return b * (dot(a, b) / dot(b, b));
}

template <typename T>
sf::Vector2<T> operator/(const sf::Vector2<T> &a, const sf::Vector2<T> &b) {
    return {a.x / b.x, a.y / b.y};
}

template <typename T>
sf::Vector2<T> operator/(T a, const sf::Vector2<T> &b) {
    return {a / b.x, a / b.y};
}

template <typename T>
sf::Vector2<T> operator*(const sf::Vector2<T> &a, const sf::Vector2<T> &b) {
    return {a.x * b.x, a.y * b.y};
}

#endif // VECTOR_OPERATIONS_H
