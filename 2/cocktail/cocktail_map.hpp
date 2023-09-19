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
    CocktailMap();
    CocktailMap(std::size_t capacity);

    CocktailMap(const CocktailMap &) = default;
    CocktailMap(CocktailMap &&) = default;

    ~CocktailMap() = default;

    CocktailMap &operator=(const CocktailMap &) = default;
    CocktailMap &operator=(CocktailMap &&) = default;

    void operator+=(const Cocktail &cocktail);
    Cocktail &operator[](const std::string &name);

    friend std::ostream &operator<<(std::ostream &stream, const CocktailMap &map);

    friend std::istream &operator>>(std::istream &stream, CocktailMap &map);

    bool is_empty() const;
    bool is_full() const;
    bool is_partially_full() const;

    float volume_with_alcohol_fraction_in_quartile(Quartile quart) const;
    float volume_with_alcohol_fraction_in_range(float min_frac, float max_frac) const;

    void rename(const std::string &old_name, const std::string &new_name);

    Cocktail mix_for_alcohol_fraction(float fraction, float volume = 5);
};

#endif  // COCKTAIL_MAP_HPP
