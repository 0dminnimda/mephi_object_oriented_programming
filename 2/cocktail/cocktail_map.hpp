#pragma once

#ifndef COCKTAIL_MAP_HPP
#define COCKTAIL_MAP_HPP

#include <iostream>
#include <string>

#include "cocktail.hpp"
#include "hash_table.hpp"

enum Quartile { FIRST, SECOND, THIRD, FOURTH };

class CocktailMap : public HashTable<std::string, Cocktail> {
public:
    /*!
    Constructs an empty map.
    */
    CocktailMap();

    /*!
    Constructs a map with the given `capacity`.
    */
    CocktailMap(std::size_t capacity);

    /*!
    Constructs a map from a given `array` of cocktails.
    */
    CocktailMap(Cocktail *array, std::size_t size);

    /*!
    Constructs a copy of the `other` map.
    */
    CocktailMap(const CocktailMap &) = default;

    /*!
    Construct-moves the `other` map.
    */
    CocktailMap(CocktailMap &&) = default;

    /*!
    Destructs the map.
    */
    ~CocktailMap() = default;

    /*!
    Copies the `other` map into this.
    */
    CocktailMap &operator=(const CocktailMap &) = default;

    /*!
    Moves the `other` map into this.
    */
    CocktailMap &operator=(CocktailMap &&) = default;

    /*!
    Inserts the `cocktail` into the map.
    */
    void operator+=(const Cocktail &cocktail);

    /*!
    Returns the cocktail with the given `name`.
    */
    Cocktail &operator[](const std::string &name);

    /*!
    Outputs the map to a `stream`. In the format `{<CocktailX>, <CocktailY>, ...}`.
    */
    friend std::ostream &operator<<(std::ostream &stream, const CocktailMap &map);

    /*!
    Inputs the cocktail from a `stream`. It reads number of cocktails, then reads each cocktail and
    inserts it into the map.
    */
    friend std::istream &operator>>(std::istream &stream, CocktailMap &map);

    /*!
    Checks if the map is empty.
    */
    bool is_empty() const;

    /*!
    Checks if the map is full.
    */
    bool is_full() const;

    /*!
    Checks if the map is partially full.
    */
    bool is_partially_full() const;

    /*!
    Returns the volume of the all cocktails with the given `alcohol_fraction` in the given quartile
    `quart`.
    */
    float volume_with_alcohol_fraction_in_quartile(Quartile quart) const;

    /*!
    Returns the volume of the all cocktails with the given `alcohol_fraction` in the given range
    [`min_frac`, `max_frac`].
    */
    float volume_with_alcohol_fraction_in_range(float min_frac, float max_frac) const;

    /*!
    Renames the cocktail in the map with the given `old_name` to the given `new_name`.
    */
    void rename(const std::string &old_name, const std::string &new_name);

    /*!
    Tries to mix the cocktail with the given `fraction` and `volume` using the cocktails in the map.
    If the cocktail can't be mixed, std::runtime_error is thrown.
    */
    Cocktail mix_for_alcohol_fraction(float fraction, float volume = 5);
};

#endif  // COCKTAIL_MAP_HPP
