#include <cassert>
#include <cstddef>
#include <functional>
#include <iostream>
#include <iterator>
#include <list>
#include <stdexcept>
#include <string>
#include <tuple>

#include "cocktail.hpp"

#define HT_NEXT_INDEX(index, capacity) (((index) + 1) % (capacity))

#define HT_FOR(index, capacity, condition)                     \
    for (std::size_t i_ = 0; (i_ < (capacity)) && (condition); \
         (++i_, index = HT_NEXT_INDEX(index, capacity)))

template <bool is_const, typename T>
using conditional_const_t = std::conditional_t<is_const, std::add_const_t<T>, T>;

template <typename Key, typename Value>
class HashTable {
public:
    template <bool is_const>
    class Iterator;

    class Entry {
    private:
        Key key;
        Value value;
        bool busy;

        friend Iterator<false>;
        friend Iterator<true>;
        friend HashTable;

    public:
        Entry() : key(), value(), busy(false) {}

        const Key &first() const { return key; }
        Value &second() { return value; }
        const Value &second() const { return value; }
    };

private:
    Entry *entries_;
    std::size_t capacity_;
    std::size_t size_;

    using Self = HashTable<Key, Value>;

public:
    HashTable() : HashTable(0) {}
    HashTable(std::size_t capacity)
        : capacity_(capacity), size_(0), entries_(new Entry[capacity]) {}
    HashTable(const Self &other)
        : capacity_(other.capacity_), size_(other.size_), entries_(new Entry[other.capacity_]) {
        std::copy_n(other.entries_, capacity_, entries_);
    }
    HashTable(Self &&other)
        : capacity_(std::exchange(other.capacity_, 0)),
          size_(std::exchange(other.size_, 0)),
          entries_(std::exchange(other.entries_, nullptr)) {}

    ~HashTable() { delete[] entries_; }

    Self &operator=(const Self &) = default;
    Self &operator=(Self &&) = default;

    bool operator==(const Self &other) const {
        return std::tie(entries_, capacity_, size_) ==
               std::tie(other.entries_, other.capacity_, other.size_);
    }
    bool operator!=(const Self &other) const { return !(*this == other); }

    std::size_t size() const { return size_; }
    std::size_t capacity() const { return capacity_; }

    void reserve(std::size_t new_capacity) {
        if (new_capacity <= capacity_) {
            return;
        }
        Entry *new_entries = new Entry[new_capacity];
        std::copy_n(entries_, capacity_, new_entries);
        delete[] entries_;
        entries_ = new_entries;
        capacity_ = new_capacity;
    }

    void copy_into(Self &table) const {
        table.reserve(table.size() + size_);
        for (const auto &it : *this) {
            table.insert(it.first(), it.second());
        }
    }

    void swap(Self &other) {
        std::swap(entries_, other.entries_);
        std::swap(capacity_, other.capacity_);
        std::swap(size_, other.size_);
    }

    void insert(const Key &key, const Value &value) {
        try_rehash();

        std::size_t index;
        if (find_index(key, index)) {
            entries_[index].value = value;
            return;
        }

        assert((index < capacity_) && "Ur fucked! Rehashing didn't help");

        entries_[index].key = key;
        entries_[index].value = value;
        entries_[index].busy = true;
        ++size_;
    }

    bool erase(const Key &key) {
        std::size_t index;
        if (find_index(key, index)) {
            entries_[index].busy = false;
            --size_;
            return true;
        }
        return false;
    }

    Value &at(const Key &key) {
        std::size_t index;
        if (find_index(key, index)) {
            return entries_[index].value;
        }
        throw std::out_of_range("Key not found");
    }

    friend std::ostream &operator<<(std::ostream &stream, const Self &table) {
        stream << "{";
        bool first = true;
        for (const auto &it : table) {
            if (!first) {
                stream << ", ";
            }
            first = false;

            stream << it.first() << ": " << it.second();
        }
        stream << "}";
        return stream;
    }

private:
    std::size_t key_index(const Key &key) const { return std::hash<Key>{}(key) % capacity_; }

    bool find_index(const Key &key, std::size_t &index) const {
        std::size_t i = key_index(key);
        HT_FOR(i, capacity_, entries_[i].busy) {
            if (entries_[i].key == key) {
                index = i;
                return true;
            }
        }
        index = i;
        return false;
    }

    void try_rehash() {
        std::size_t new_capacity = (size_ + 1) * 2;
        if (new_capacity <= capacity_) return;

        Self new_table(new_capacity);

        for (const auto &it : *this) {
            new_table.insert(it.first(), it.second());
        }

        new_table.swap(*this);
    }

public:
    template <bool is_const>
    class Iterator {
    private:
        using iterator_category = std::forward_iterator_tag;
        using value_type = conditional_const_t<is_const, Entry>;
        using difference_type = std::ptrdiff_t;
        using pointer = value_type *;
        using reference = value_type &;

        using TableT = conditional_const_t<is_const, HashTable<Key, Value>>;
        TableT &table;
        std::size_t index;

    public:
        Iterator(TableT &table, std::size_t i) : table(table), index(i) { skip_untill_busy(); }
        Iterator(const Iterator &) = default;
        Iterator &operator=(const Iterator &) = default;

        bool operator==(const Iterator &other) const {
            return std::tie(table, index) == std::tie(other.table, other.index);
        }
        bool operator!=(const Iterator &other) const { return !(*this == other); }

        Iterator &operator++() {
            progress();
            skip_untill_busy();
            return *this;
        }

        Iterator operator++(int) {
            Iterator result(*this);
            ++(*this);
            return result;
        }

        reference operator*() const { return table.entries_[index]; }

    private:
        void progress() {
            if (index < table.capacity_) {
                ++index;
            }
        }

        void skip_untill_busy() {
            while (index < table.capacity_ && !table.entries_[index].busy) {
                ++index;
            }
        }
    };

    Iterator<false> begin() { return Iterator<false>(*this, 0); }
    Iterator<false> end() { return Iterator<false>(*this, capacity_); }

    Iterator<true> begin() const { return Iterator<true>(*this, 0); }
    Iterator<true> end() const { return Iterator<true>(*this, capacity_); }

    Iterator<true> cbegin() const { return Iterator<true>(*this, 0); }
    Iterator<true> cend() const { return Iterator<true>(*this, capacity_); }
};

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
        CocktailMap temp;
        this->copy_into(temp);

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
