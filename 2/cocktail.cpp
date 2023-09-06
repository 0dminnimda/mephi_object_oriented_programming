#include <iostream>
#include <stdexcept>
#include <string>

using std::to_string;

class Cocktail {
private:
    float volume_;
    float alcohol_fraction_;

public:
    Cocktail() noexcept = default;
    Cocktail(float volume, float alcohol_fraction) {
        this->volume(volume);
        this->alcohol_fraction(alcohol_fraction);
    }
    Cocktail(float volume) : Cocktail(volume, 0) {}

    Cocktail(const Cocktail &) noexcept = default;
    Cocktail& operator=(const Cocktail &) noexcept = default;

    static float valid_volume(float value) {
        if (0 <= value)
            return value;
        throw std::runtime_error("Volume should be positive");
    }

    float volume() const noexcept { return volume_; }
    float volume(float value) {
        return volume_ = valid_volume(value);
    }

    static float valid_alcohol_fraction(float value) {
        if (0 <= value && value <= 1)
            return value;
        throw std::runtime_error("Alcohol fraction should be in range [0, 1]");
    }

    float alcohol_fraction() const noexcept { return alcohol_fraction_; }
    float alcohol_fraction(float value) {
        return alcohol_fraction_ = valid_alcohol_fraction(value);
    }

    float alcohol_volume() const noexcept { return volume() * alcohol_fraction(); }
    float water_volume() const noexcept { return volume() - alcohol_volume(); }

    Cocktail& operator+=(const Cocktail& other) noexcept {
        if (!other.is_empty()) {
            float total_alcohol_volume = alcohol_volume() + other.alcohol_volume();
            volume(volume() + other.volume());
            alcohol_fraction(total_alcohol_volume / volume());
        }
        return *this;
    }

    Cocktail operator+(const Cocktail &other) const noexcept {
        Cocktail result(*this);
        result += other;
        return result;
    }

    template <typename T>
    Cocktail& operator*=(T other) noexcept {
        volume(volume() * other);
        return *this;
    }

    template <typename T>
    Cocktail operator*(T other) const noexcept {
        Cocktail result(*this);
        result *= other;
        return result;
    }

    bool is_empty() const noexcept {
        return volume() == 0;
    }

    void empty() noexcept {
        volume(0);
        alcohol_fraction(0);
    }

    Cocktail split(float part_volume) {
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

    void pour(Cocktail& other, float poured_volume) {
        other += split(poured_volume);
    }

    Cocktail& operator>>(Cocktail &other) {
        pour(other, 1);
        return *this;
    }

    friend std::string to_string(const Cocktail &cock) {
        std::string result;
        result += "Cocktail(";
        result += "volume=" + to_string(cock.volume()) + ", ";
        result += "alcohol_fraction=" + to_string(cock.alcohol_fraction());
        result += ")";
        return result;
    }

    friend std::ostream& operator<<(std::ostream &stream, const Cocktail &cock) {
        stream << to_string(cock);
        return stream;
    }

    friend std::istream& operator>>(std::istream &stream, Cocktail &cock) {
        float volume;
        stream >> volume;

        if (!stream.good()) { goto bad; }

        float alcohol_fraction;
        stream >> alcohol_fraction;

        if (!stream.good()) { goto bad; }

        cock.volume(volume);
        cock.alcohol_fraction(alcohol_fraction);

        return stream;

bad:
        stream.setstate(std::ios::failbit);
        return stream;
    }
};
