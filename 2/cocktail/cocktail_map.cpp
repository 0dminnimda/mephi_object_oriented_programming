#include <functional>
#include <iostream>
#include <iterator>
#include <list>
#include <string>
#include <tuple>

#include "cocktail.hpp"
#include "hash_table.hpp"

enum Quartile { FIRST, SECOND, THIRD, FOURTH };

class CocktailMap : public HashTable<std::string, Cocktail> {
public:
    CocktailMap() : HashTable() {}
    CocktailMap(std::size_t capacity) : HashTable(capacity) {}

    CocktailMap(const CocktailMap &) = default;
    CocktailMap(CocktailMap &&) = default;

    ~CocktailMap() = default;

    CocktailMap &operator=(const CocktailMap &) = default;
    CocktailMap &operator=(CocktailMap &&) = default;

    void operator+=(const Cocktail &cocktail) { insert(cocktail.name(), cocktail); }
    Cocktail &operator[](const std::string &name) { return at(name); }

    friend std::ostream &operator<<(std::ostream &stream, const CocktailMap &table) {
        stream << "{";
        bool first = true;
        for (const auto &it : table) {
            if (!first) {
                stream << ", ";
            }
            first = false;

            stream << it.second();
        }
        stream << "}";
        return stream;
    }

    bool is_empty() const { return size() <= 0; }
    bool is_full() const { return size() >= capacity(); }
    bool is_partially_full() const { return !is_empty() && !is_full(); }

    float volume_with_alcohol_fraction_in_quartile(Quartile quart) const {
        switch (quart) {
            case FIRST: return volume_with_alcohol_fraction_in_range(0, 0.25);
            case SECOND: return volume_with_alcohol_fraction_in_range(0.25, 0.5);
            case THIRD: return volume_with_alcohol_fraction_in_range(0.5, 0.75);
            case FOURTH: return volume_with_alcohol_fraction_in_range(0.75, 1);
            default: return 0;
        }
    }

    float volume_with_alcohol_fraction_in_range(float min_frac, float max_frac) const {
        float result = 0;
        for (const auto &it : *this) {
            float frac = it.second().alcohol_fraction();
            if (min_frac <= frac && frac <= max_frac) {
                result += it.second().volume();
            }
        }
        return result;
    }

    void rename(const std::string &old_name, const std::string &new_name) {
        Cocktail cock = at(old_name);
        erase(old_name);
        cock.name(new_name);
        (*this) += cock;
    }

    Cocktail mix_for_alcohol_fraction(float fraction, float volume = 5) {
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

private:
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
};
