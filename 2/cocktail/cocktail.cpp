#include "cocktail.hpp"

#include <iostream>
#include <stdexcept>
#include <string>
#include <tuple>

using std::to_string;

Cocktail::Cocktail() noexcept { empty(); }

Cocktail::Cocktail(const std::string &name, float volume, float alcohol_fraction) {
    this->name(name);
    this->volume(volume);
    this->alcohol_fraction(alcohol_fraction);
    if (!volume) empty();
}

Cocktail::Cocktail(const std::string &name, float volume) : Cocktail(name, volume, 0) {}

bool Cocktail::operator==(const Cocktail &other) const noexcept {
    return std::tie(name_, volume_, alcohol_fraction_) ==
           std::tie(other.name_, other.volume_, other.alcohol_fraction_);
}

bool Cocktail::operator!=(const Cocktail &other) const noexcept { return !(*this == other); }

const std::string &Cocktail::valid_name(const std::string &value) { return value; }

const std::string &Cocktail::name() const noexcept { return name_; }

const std::string &Cocktail::name(const std::string &value) { return name_ = valid_name(value); }

float Cocktail::valid_volume(float value) {
    if (0 <= value) return value;
    throw std::runtime_error("Volume should be positive");
}

float Cocktail::volume() const noexcept { return volume_; }

float Cocktail::volume(float value) { return volume_ = valid_volume(value); }

float Cocktail::valid_alcohol_fraction(float value) {
    if (0 <= value && value <= 1) return value;
    throw std::runtime_error("Alcohol fraction should be in range [0, 1]");
}

float Cocktail::alcohol_fraction() const noexcept { return alcohol_fraction_; }

float Cocktail::alcohol_fraction(float value) {
    return alcohol_fraction_ = valid_alcohol_fraction(value);
}

float Cocktail::alcohol_volume() const noexcept { return volume() * alcohol_fraction(); }

float Cocktail::water_volume() const noexcept { return volume() - alcohol_volume(); }

Cocktail &Cocktail::operator+=(const Cocktail &other) noexcept {
    if (!other.is_empty()) {
        float total_alcohol_volume = alcohol_volume() + other.alcohol_volume();
        volume(volume() + other.volume());
        alcohol_fraction(total_alcohol_volume / volume());
    }
    name(name() + other.name());
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

bool Cocktail::is_empty() const noexcept { return volume() == 0; }

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

void Cocktail::pour(Cocktail &other, float poured_volume) { other += split(poured_volume); }

const float Cocktail::volume_ratio_for_sum(
    float left_fraction, float right_fraction, float target_fraction
) {
    return (target_fraction - right_fraction) / (left_fraction - right_fraction);
}

const std::pair<float, float> Cocktail::volumes_for_sum(
    float left_fraction, float right_fraction, float target_fraction, float total_volume
) {
    float volume_ratio = volume_ratio_for_sum(left_fraction, right_fraction, target_fraction);
    float first_vomue = total_volume * volume_ratio;
    float second_vomue = total_volume - first_vomue;
    return std::make_pair(first_vomue, second_vomue);
}

bool Cocktail::mix_for_alcohol_fraction_sorted(
    Cocktail &min_fraction_cocktail, Cocktail &max_fraction_cocktail, Cocktail &result,
    float target_fraction, float volume_to_add
) {
    auto [first_volume, second_volume] = volumes_for_sum(
        min_fraction_cocktail.alcohol_fraction(), max_fraction_cocktail.alcohol_fraction(),
        target_fraction, volume_to_add
    );

    bool ok = true;
    if (min_fraction_cocktail.volume() < first_volume ||
        max_fraction_cocktail.volume() < second_volume)
    {
        ok = false;
        float scale = std::min(
            min_fraction_cocktail.volume() / first_volume,
            max_fraction_cocktail.volume() / second_volume
        );
        first_volume *= scale;
        second_volume *= scale;
    }

    min_fraction_cocktail.pour(result, first_volume);
    max_fraction_cocktail.pour(result, second_volume);

    return ok;
}

bool Cocktail::mix_for_alcohol_fraction(
    Cocktail &lhs, Cocktail &rhs, Cocktail &result, float target_fraction, float volume_to_add
) {
    if (lhs.alcohol_fraction() < rhs.alcohol_fraction()) {
        return mix_for_alcohol_fraction_sorted(lhs, rhs, result, target_fraction, volume_to_add);
    } else {
        return mix_for_alcohol_fraction_sorted(rhs, lhs, result, target_fraction, volume_to_add);
    }
}

Cocktail &Cocktail::operator>>(Cocktail &other) {
    pour(other, 1);
    return *this;
}

Cocktail &Cocktail::operator<<(Cocktail &other) {
    other.pour(*this, 1);
    return *this;
}

std::string Cocktail::to_string() const {
    std::string result;
    result += "Cocktail(";
    result += "name=\"" + name() + "\", ";
    result += "volume=" + std::to_string(volume()) + ", ";
    result += "alcohol_fraction=" + std::to_string(alcohol_fraction());
    result += ")";
    return result;
}

std::string std::to_string(const Cocktail &cock) { return cock.to_string(); }

std::ostream &operator<<(std::ostream &stream, const Cocktail &cock) {
    stream << to_string(cock);
    return stream;
}

std::istream &operator>>(std::istream &stream, Cocktail &cock) {
    std::string name;
    stream >> name;

    if (!stream.good()) {
        goto bad;
    }

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

    cock.name(name);
    cock.volume(volume);
    cock.alcohol_fraction(alcohol_fraction);

    return stream;

bad:
    stream.setstate(std::ios::failbit);
    return stream;
}
