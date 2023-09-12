#include "cocktail.hpp"

#include <iostream>
#include <stdexcept>
#include <string>
#include <tuple>


using std::to_string;

Cocktail::Cocktail() noexcept {
    empty();
}

Cocktail::Cocktail(float volume, float alcohol_fraction) {
    this->volume(volume);
    this->alcohol_fraction(alcohol_fraction);
    if (!volume) empty();
}

Cocktail::Cocktail(float volume) : Cocktail(volume, 0) {}

bool Cocktail::operator==(const Cocktail &other) const noexcept {
    return std::tie(volume_, alcohol_fraction_) ==
           std::tie(other.volume_, other.alcohol_fraction_);
}

bool Cocktail::operator!=(const Cocktail &other) const noexcept {
    return !(*this == other);
}

float Cocktail::valid_volume(float value) {
    if (0 <= value) return value;
    throw std::runtime_error("Volume should be positive");
}

float Cocktail::volume() const noexcept {
    return volume_;
}

float Cocktail::volume(float value) {
    return volume_ = valid_volume(value);
}

float Cocktail::valid_alcohol_fraction(float value) {
    if (0 <= value && value <= 1) return value;
    throw std::runtime_error("Alcohol fraction should be in range [0, 1]");
}

float Cocktail::alcohol_fraction() const noexcept {
    return alcohol_fraction_;
}

float Cocktail::alcohol_fraction(float value) {
    return alcohol_fraction_ = valid_alcohol_fraction(value);
}

float Cocktail::alcohol_volume() const noexcept {
    return volume() * alcohol_fraction();
}

float Cocktail::water_volume() const noexcept {
    return volume() - alcohol_volume();
}

Cocktail &Cocktail::operator+=(const Cocktail &other) noexcept {
    if (!other.is_empty()) {
        float total_alcohol_volume = alcohol_volume() + other.alcohol_volume();
        volume(volume() + other.volume());
        alcohol_fraction(total_alcohol_volume / volume());
    }
    return *this;
}

Cocktail Cocktail::operator+(const Cocktail &other) const noexcept {
    Cocktail result(*this);
    result += other;
    return result;
}

template <typename T>
Cocktail &Cocktail::operator*=(T other) noexcept {
    volume(volume() * other);
    return *this;
}

// Fuck this lab's task requirements
template Cocktail &Cocktail::operator*=(short other) noexcept;
template Cocktail &Cocktail::operator*=(int other) noexcept;
template Cocktail &Cocktail::operator*=(long other) noexcept;
template Cocktail &Cocktail::operator*=(long long other) noexcept;
template Cocktail &Cocktail::operator*=(unsigned short other) noexcept;
template Cocktail &Cocktail::operator*=(unsigned int other) noexcept;
template Cocktail &Cocktail::operator*=(unsigned long other) noexcept;
template Cocktail &Cocktail::operator*=(unsigned long long other) noexcept;
template Cocktail &Cocktail::operator*=(float other) noexcept;
template Cocktail &Cocktail::operator*=(double other) noexcept;
template Cocktail &Cocktail::operator*=(long double other) noexcept;

template <typename T>
Cocktail Cocktail::operator*(T other) const noexcept {
    Cocktail result(*this);
    result *= other;
    return result;
}

// Fuck this lab's task requirements
template Cocktail Cocktail::operator*(short other) const noexcept;
template Cocktail Cocktail::operator*(int other) const noexcept;
template Cocktail Cocktail::operator*(long other) const noexcept;
template Cocktail Cocktail::operator*(long long other) const noexcept;
template Cocktail Cocktail::operator*(unsigned short other) const noexcept;
template Cocktail Cocktail::operator*(unsigned int other) const noexcept;
template Cocktail Cocktail::operator*(unsigned long other) const noexcept;
template Cocktail Cocktail::operator*(unsigned long long other) const noexcept;
template Cocktail Cocktail::operator*(float other) const noexcept;
template Cocktail Cocktail::operator*(double other) const noexcept;
template Cocktail Cocktail::operator*(long double other) const noexcept;

bool Cocktail::is_empty() const noexcept {
    return volume() == 0;
}

void Cocktail::empty() noexcept {
    volume(0);
    alcohol_fraction(0);
}

Cocktail Cocktail::split(float part_volume) {
    valid_volume(part_volume);
    Cocktail part(*this);
    if (volume() < part_volume) {
        empty();
    } else {
        part *= (part_volume / volume());
        volume(volume() - part_volume);
    }
    return part;
}

void Cocktail::pour(Cocktail &other, float poured_volume) {
    other += split(poured_volume);
}

Cocktail &Cocktail::operator>>(Cocktail &other) {
    pour(other, 1);
    return *this;
}

std::string to_string(const Cocktail &cock) {
    std::string result;
    result += "Cocktail(";
    result += "volume=" + to_string(cock.volume()) + ", ";
    result += "alcohol_fraction=" + to_string(cock.alcohol_fraction());
    result += ")";
    return result;
}

std::ostream &operator<<(std::ostream &stream, const Cocktail &cock) {
    stream << to_string(cock);
    return stream;
}

std::istream &operator>>(std::istream &stream, Cocktail &cock) {
    float volume;
    stream >> volume;

    if (!stream.good()) {
        goto bad;
    }

    float alcohol_fraction;
    stream >> alcohol_fraction;

    if (!stream.good()) {
        goto bad;
    }

    cock.volume(volume);
    cock.alcohol_fraction(alcohol_fraction);

    return stream;

bad:
    stream.setstate(std::ios::failbit);
    return stream;
}
