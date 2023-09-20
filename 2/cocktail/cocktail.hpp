#pragma once

#ifndef COCKTAIL_HPP
#define COCKTAIL_HPP

#include <iostream>
#include <string>
#include <utility>

class Cocktail {
private:
    std::string name_;        /// Name of the cocktail. Used for identification.
    float volume_;            /// Total volume of the cocktail.
    float alcohol_fraction_;  /// How much of the volume is alcohol.

public:
    /*!
    Constructs an empty cocktail.
    */
    Cocktail() noexcept;

    /*!
    Constructs a cocktail from the `name`, `volume` and `alcohol_fraction`.
    */
    Cocktail(const std::string &name, float volume, float alcohol_fraction);

    /*!
    Constructs a cocktail from the `name` and `volume`.
    */
    Cocktail(const std::string &name, float volume);

    /*!
    Constructs a copy of the `other` cocktail.
    */
    Cocktail(const Cocktail &) = default;

    /*!
    Copies the `other` cocktail into this.
    */
    Cocktail &operator=(const Cocktail &) = default;

    /*!
    Checks if the `other` cocktail is equal to this one.
    */
    bool operator==(const Cocktail &other) const noexcept;

    /*!
    Checks if the `other` cocktail is not equal to this one.
    */
    bool operator!=(const Cocktail &other) const noexcept;

    /*!
    Checks if the `value` is valid, currently all strings are valid.
    */
    static const std::string &valid_name(const std::string &value);

    /*!
    Getter for name.
    */
    const std::string &name() const noexcept;

    /*!
    Setter for name. `value` is validated by `valid_name`.
    */
    const std::string &name(const std::string &value);

    /*!
    Checks if the `value` is positive, if true returns it, otherwise throws
    std::runtime_error.
    */
    static float valid_volume(float value);

    /*!
    Getter for volume.
    */
    float volume() const noexcept;

    /*!
    Setter for volume. `value` is validated by `valid_volume`.
    */
    float volume(float value);

    /*!
    Checks if the `value` is in range [0, 1], if true returns it, otherwise throws
    std::runtime_error.
    */
    static float valid_alcohol_fraction(float value);

    /*!
    Getter for alcohol fraction.
    */
    float alcohol_fraction() const noexcept;

    /*!
    Setter for alcohol fraction. `value` is validated by `valid_alcohol_fraction`.
    */
    float alcohol_fraction(float value);

    /*!
    Returns the alcohol volume of the cocktail, excluding water.
    */
    float alcohol_volume() const noexcept;

    /*!
    Returns the water volume of the cocktail, excluding alcohol.
    */
    float water_volume() const noexcept;

    /*!
    Will mix `other` cocktail into this cocktail.
    */
    Cocktail &operator+=(const Cocktail &other) noexcept;

    /*!
    Will return the mix of this cocktail and `other` cocktail.
    */
    Cocktail operator+(const Cocktail &other) const noexcept;

    /*!
    Will multiply the volume of the cocktail by `other`. `T` have to be able to be multiplied with a
    float.
    */
    template <typename T>
    Cocktail &operator*=(T other) noexcept;

    /*!
    Will create a copy of the cocktail with the volume multiplied by `other`. `T` have to be able to
    be multiplied with a float.
    */
    template <typename T>
    Cocktail operator*(T other) const noexcept;

    /*!
    Will check if the cocktail is empty. Equivalent to checking if the volume is 0.
    */
    bool is_empty() const noexcept;

    /*!
    Will make the cocktail empty, set volume to 0 and alcohol fraction to 0.
    */
    void empty() noexcept;

    /*!
    Will remove the part of the cocktail and return it. The volume of the returned part will be
    `part_volume`.
    */
    Cocktail split(float part_volume);

    /*!
    Will remove the part of the cocktail and add it to the `other`. The volume of the added part
    will be `poured_volume`.
    */
    void pour(Cocktail &other, float poured_volume);

    /*!
    Given two cocktails with fractions `left_fraction` and `right_fraction`, and expecting the
    fraction of thier sum to be `target_fraction`, will return the volume ratio two
    cocktails should have.
    */
    static const float volume_ratio_for_sum(
        float left_fraction, float right_fraction, float target_fraction
    );

    /*!
    Given two cocktails with fractions `left_fraction` and `right_fraction`, and expecting the
    fraction and volume of thier sum to be `target_fraction` and `total_volume`, will return
    the volumes two cocktails should have.
    */
    static const std::pair<float, float> volumes_for_sum(
        float left_fraction, float right_fraction, float target_fraction, float total_volume
    );

    /*!
    Given two cocktails `lhs` and `rhs` will mix a cocktail with fraction `target_fraction` and
    volume of `volume_to_add` or less*, and add it to `result`. *If the ither of the cocktails don't
    have enough volume, most possible amount will be taken, in this case the return valse is false,
    otherwise it's true.
    */
    static bool mix_for_alcohol_fraction(
        Cocktail &lhs, Cocktail &rhs, Cocktail &result, float target_fraction, float volume_to_add
    );

    /*!
    Same as `mix_for_alcohol_fraction`, but `min_fraction_cocktail` is required to have smaller
    alcohol fraction than `max_fraction_cocktail`.
    @see mix_for_alcohol_fraction
    */
    static bool mix_for_alcohol_fraction_sorted(
        Cocktail &min_fraction_cocktail, Cocktail &max_fraction_cocktail, Cocktail &result,
        float target_fraction, float volume_to_add
    );

    /*!
    Pours 1 unit of volume of this cocktail into the `other` cocktail.
    */
    Cocktail &operator>>(Cocktail &other);

    /*!
    Pours 1 unit of volume of the `other` cocktail into this cocktail.
    */
    Cocktail &operator<<(Cocktail &other);

    /*!
    Creates a `string` representation of the cocktail.
    */
    std::string to_string() const;

    /*!
    Outputs the cocktail to a `stream`.
    */
    friend std::ostream &operator<<(std::ostream &stream, const Cocktail &cock);

    /*!
    Inputs the cocktail from a `stream`. It reads each field and sets it to the cocktail.
    */
    friend std::istream &operator>>(std::istream &stream, Cocktail &cock);
};

namespace std {
    std::string to_string(const Cocktail &cock);
}

#endif  // COCKTAIL_HPP
