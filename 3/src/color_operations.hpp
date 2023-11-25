#pragma once

#ifndef COLOR_OPERATIONS_H
#define COLOR_OPERATIONS_H

#include <SFML/Graphics.hpp>
#include <algorithm>
#include <ostream>

sf::Color operator*(const sf::Color &a, const float b) {
    return sf::Color(
        std::min(a.r * b, 255.0f), std::min(a.g * b, 255.0f), std::min(a.b * b, 255.0f),
        std::min(a.a * b, 255.0f)
    );
}

namespace std {
    std::ostream &operator<<(std::ostream &stream, const sf::Color &a) {
        stream << "Color(" << a.r << ", " << a.g << ", " << a.b << ", " << a.a << ")";
        return stream;
    }
}  // namespace std

#endif  // COLOR_OPERATIONS_H
