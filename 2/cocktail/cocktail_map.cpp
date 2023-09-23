#include "cocktail_map.hpp"

#include <functional>
#include <list>

CocktailMap::CocktailMap() : HashTable() {}

CocktailMap::CocktailMap(std::size_t capacity) : HashTable(capacity) {}

CocktailMap::CocktailMap(Cocktail *array, std::size_t size) : HashTable(size) {
    for (std::size_t i = 0; i < size; ++i) {
        *this += array[i];
    }
}

CocktailMap &CocktailMap::operator+=(const Cocktail &cocktail) {
    insert(cocktail.name(), cocktail);
    return *this;
}

Cocktail &CocktailMap::operator[](const std::string &name) { return at(name); }
const Cocktail &CocktailMap::operator[](const std::string &name) const { return at(name); }

std::ostream &operator<<(std::ostream &stream, const CocktailMap &map) {
    stream << "{";
    bool first = true;
    for (const auto &it : map) {
        if (!first) {
            stream << ", ";
        }
        first = false;

        stream << it.second();
    }
    stream << "}";
    return stream;
}

std::istream &operator>>(std::istream &stream, CocktailMap &map) {
    std::size_t n = 0;
    stream >> n;

    for (size_t i = 0; i < n; ++i) {
        Cocktail cock;
        stream >> cock;
        if (stream.fail()) {
            stream.setstate(std::ios::failbit);
            break;
        }
        map += cock;
    }

    return stream;
}

bool CocktailMap::is_empty() const { return size() <= 0; }
bool CocktailMap::is_full() const { return size() >= capacity(); }
bool CocktailMap::is_partially_full() const { return !is_empty() && !is_full(); }

float CocktailMap::volume_with_alcohol_fraction_in_quartile(Quartile quart) const {
    switch (quart) {
        case FIRST: return volume_with_alcohol_fraction_in_range(0, 0.25);
        case SECOND: return volume_with_alcohol_fraction_in_range(0.25, 0.5);
        case THIRD: return volume_with_alcohol_fraction_in_range(0.5, 0.75);
        case FOURTH: return volume_with_alcohol_fraction_in_range(0.75, 1);
        default: return 0;
    }
}

float CocktailMap::volume_with_alcohol_fraction_in_range(float min_frac, float max_frac) const {
    float result = 0;
    for (const auto &it : *this) {
        float frac = it.second().alcohol_fraction();
        if (min_frac <= frac && frac <= max_frac) {
            result += it.second().volume();
        }
    }
    return result;
}

void CocktailMap::rename(const std::string &old_name, const std::string &new_name) {
    Cocktail cock = at(old_name);
    erase(old_name);
    cock.name(new_name);
    (*this) += cock;
}

static bool mix_for_alcohol_fraction_no_map_(
    float fraction, float volume, Cocktail &result,
    std::list<std::reference_wrapper<Cocktail>> &cocktails
) {
    while (cocktails.size() > 0 && cocktails.front().get().volume() == 0) cocktails.pop_front();
    while (cocktails.size() > 0 && cocktails.back().get().volume() == 0) cocktails.pop_back();

    if (cocktails.size() <= 0) return false;

    Cocktail &min_frac = cocktails.front().get();
    Cocktail &max_frac = cocktails.back().get();

    float volume_to_add = volume - result.volume();
    bool ok = Cocktail::mix_for_alcohol_fraction_sorted(
        min_frac, max_frac, result, fraction, volume_to_add
    );
    if (!ok) {
        ok = mix_for_alcohol_fraction_no_map_(fraction, volume, result, cocktails);
    }
    return ok;
}

Cocktail CocktailMap::mix_for_alcohol_fraction(float fraction, float volume) {
    CocktailMap temp = *this;

    std::list<std::reference_wrapper<Cocktail>> cocktails;
    for (auto &it : temp) {
        cocktails.push_back(it.second());
    }

    cocktails.sort([](const Cocktail &a, const Cocktail &b) {
        return a.alcohol_fraction() < b.alcohol_fraction();
    });

    Cocktail result;
    if (!mix_for_alcohol_fraction_no_map_(fraction, volume, result, cocktails)) {
        throw std::runtime_error("Could not make you such cocktail, sorry!");
    }

    temp.swap(*this);
    return result;
}
