#ifndef COCKTAIL_HPP
#define COCKTAIL_HPP

#pragma once

#include <iostream>
#include <string>
#include <utility>

class Cocktail {
private:
    std::string name_;
    float volume_;
    float alcohol_fraction_;

public:
    Cocktail() noexcept;

    Cocktail(const std::string &name, float volume, float alcohol_fraction);

    Cocktail(const std::string &name, float volume);

    Cocktail(const Cocktail &) = default;

    Cocktail &operator=(const Cocktail &) = default;

    bool operator==(const Cocktail &other) const noexcept;

    bool operator!=(const Cocktail &other) const noexcept;

    static const std::string &valid_name(const std::string &value);

    const std::string &name() const noexcept;

    const std::string &name(const std::string &value);

    static float valid_volume(float value);

    float volume() const noexcept;

    float volume(float value);

    static float valid_alcohol_fraction(float value);

    float alcohol_fraction() const noexcept;

    float alcohol_fraction(float value);

    float alcohol_volume() const noexcept;

    float water_volume() const noexcept;

    Cocktail &operator+=(const Cocktail &other) noexcept;

    Cocktail operator+(const Cocktail &other) const noexcept;

    template <typename T>
    Cocktail &operator*=(T other) noexcept;

    template <typename T>
    Cocktail operator*(T other) const noexcept;

    bool is_empty() const noexcept;

    void empty() noexcept;

    Cocktail split(float part_volume);

    void pour(Cocktail &other, float poured_volume);

    static const float volume_ratio_for_sum(
        float left_fraction, float right_fraction, float target_fraction
    );

    static const std::pair<float, float> volumes_for_sum(
        float left_fraction, float right_fraction, float target_fraction, float total_volume
    );

    static bool mix_for_alcohol_fraction(
        Cocktail &lhs, Cocktail &rhs, Cocktail &result, float target_fraction, float volume_to_add
    );

    static bool mix_for_alcohol_fraction_sorted(
        Cocktail &min_fraction_cocktail, Cocktail &max_fraction_cocktail, Cocktail &result,
        float target_fraction, float volume_to_add
    );

    Cocktail &operator>>(Cocktail &other);

    std::string to_string() const;

    friend std::ostream &operator<<(std::ostream &stream, const Cocktail &cock);

    friend std::istream &operator>>(std::istream &stream, Cocktail &cock);
};

namespace std {
    std::string to_string(const Cocktail &cock);
}

#endif  // COCKTAIL_HPP
