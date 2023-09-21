#pragma once

#ifndef HASH_TABLE_HPP
#define HASH_TABLE_HPP

#include <cassert>
#include <cstddef>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <type_traits>

#include "array.hpp"

#ifndef HT_NEXT_INDEX
#define HT_NEXT_INDEX(index, capacity) (((index) + 1) % (capacity))
#endif  // HT_NEXT_INDEX

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
        Key key;      /// The thing value is associated with.
        Value value;  /// The mapped value that HashTable stores.
        bool busy;    /// Indicates if the entry can be set or not.

        friend Iterator<false>;
        friend Iterator<true>;
        friend HashTable;

    public:
        /*!
        Constructs an empty entry.
        */
        Entry() : key(), value(), busy(false) {}

        /*!
        Getter for key. Emulates std::pair.
        */
        const Key &first() const { return key; }

        /*!
        Getter for value. Emulates std::pair.
        */
        Value &second() { return value; }

        /*!
        Getter for value. Emulates std::pair.
        */
        const Value &second() const { return value; }

        /*!
        Checks if the `other` entry is equal to this one.
        */
        bool operator==(const Entry &other) const {
            if (busy != other.busy) return false;
            if (busy == false && other.busy == false) return true;
            return std::tie(key, value) == std::tie(other.key, other.value);
        }

        /*!
        Checks if the `other` entry is not equal to this one.
        */
        bool operator!=(const Entry &other) const { return !(*this == other); }
    };

private:
    Array<Entry> entries_;  /// Array of the entries, the core of the HashTable.

    using Self = HashTable<Key, Value>;

public:
    /*!
    Constructs an empty HashTable.
    */
    HashTable() : HashTable(0) {}

    /*!
    Constructs a HashTable with the given `capacity`.
    */
    HashTable(std::size_t capacity) : entries_(capacity, 0) {}

    /*!
    Constructs a copy of the `other` HashTable.
    */
    HashTable(const Self &other) = default;

    /*!
    Construct-moves the `other` HashTable.
    */
    HashTable(Self &&other) = default;

    /*!
    Destructs the HashTable.
    */
    ~HashTable() = default;

    /*!
    Copies the `other` HashTable into this.
    */
    Self &operator=(const Self &) = default;

    /*!
    Moves the `other` HashTable into this.
    */
    Self &operator=(Self &&) = default;

    /*!
    Checks if the `other` HashTable is equal to this one.
    */
    bool operator==(const Self &other) const { return entries_ == other.entries_; }

    /*!
    Checks if the `other` HashTable is not equal to this one.
    */
    bool operator!=(const Self &other) const { return !(*this == other); }

    /*!
    Returns the amount of the entries in the HashTable.
    */
    std::size_t size() const { return entries_.size; }

    /*!
    Returns the capacity of the entries in the HashTable. How many more of the entries could be
    added without a need to resize and rehash.
    */
    std::size_t capacity() const { return entries_.capacity; }

    /*!
    Exchanges the contents of this HashTable and `other` HashTable. Does not create copies.
    */
    void swap(Self &other) { std::swap(entries_, other.entries_); }

    /*!
    Adds the mapping of the `key` to the `value`.
    */
    void insert(const Key &key, const Value &value) {
        try_rehash();

        std::size_t index;
        if (find_index(key, index)) {
            entries_[index].value = value;
            return;
        }

        assert((index < capacity()) && "Ur fucked! Rehashing didn't help");

        entries_[index].key = key;
        entries_[index].value = value;
        entries_[index].busy = true;
        ++entries_.size;
    }

    /*!
    Removes the mapping of the `key`.
    */
    bool erase(const Key &key) {
        std::size_t index;
        if (find_index(key, index)) {
            entries_[index].busy = false;
            --entries_.size;
            return true;
        }
        return false;
    }

    /*!
    Checks if the `key` is in the HashTable.
    */
    bool contains(const Key &key) {
        std::size_t index;
        return find_index(key, index);
    }

    /*!
    Returns the value associated with the `key`.
    */
    Value &at(const Key &key) {
        std::size_t index;
        if (find_index(key, index)) {
            return entries_[index].value;
        }
        throw std::out_of_range("Key not found");
    }

    /*!
    Outputs the HashTable to a `stream`. In the format `{<keyX>: <valueX>, <keyY>: <valueY>, ...}`.
    */
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
    std::size_t key_index(const Key &key) const { return std::hash<Key>{}(key) % capacity(); }

    bool find_index(const Key &key, std::size_t &index) const {
        std::size_t i = key_index(key);
        HT_FOR(i, capacity(), entries_[i].busy) {
            if (entries_[i].key == key) {
                index = i;
                return true;
            }
        }
        index = i;
        return false;
    }

    void try_rehash() {
        std::size_t new_capacity = (size() + 1) * 2;
        if (new_capacity <= capacity()) return;

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
        TableT &table;      /// The reference to the HashTable that is being iterated.
        std::size_t index;  /// The index of the current entry. Points to the busy entry or to the
                            /// end of the HashTable - out of bounds of the entries array.

    public:
        /*!
        Disallows the default iterator, which consequently iterates nothing.
        */
        Iterator() = delete;

        /*!
        Constructs an iterator for the `table` starting at the first busy index after or at `i`.
        */
        Iterator(TableT &table, std::size_t i) : table(table), index(i) { skip_untill_busy(); }

        /*!
        Constructs a copy of the other iterator.
        */
        Iterator(const Iterator &) = default;

        /*!
        Copies the other iterator into this.
        */
        Iterator &operator=(const Iterator &) = default;

        /*!
        Checks if the `other` iterator is equal to this one.
        */
        bool operator==(const Iterator &other) const {
            return std::tie(table, index) == std::tie(other.table, other.index);
        }

        /*!
        Checks if the `other` iterator is not equal to this one.
        */
        bool operator!=(const Iterator &other) const { return !(*this == other); }

        /*!
        Advances the iterator to the next busy entry.
        */
        Iterator &operator++() {
            progress();
            skip_untill_busy();
            return *this;
        }

        /*!
        Returns a copy of this iterator and advances it to the next busy entry.
        */
        Iterator operator++(int) {
            Iterator result(*this);
            ++(*this);
            return result;
        }

        /*!
        Returns a reference to the current entry.
        */
        reference operator*() const { return table.entries_[index]; }

    private:
        void progress() {
            if (index < table.capacity()) {
                ++index;
            }
        }

        void skip_untill_busy() {
            while (index < table.capacity() && !table.entries_[index].busy) {
                ++index;
            }
        }
    };

    /*!
    Returns an iterator to the first entry in the HashTable.
    */
    Iterator<false> begin() { return Iterator<false>(*this, 0); }

    /*!
    Returns an iterator to the last entry in the HashTable.
    */
    Iterator<false> end() { return Iterator<false>(*this, capacity()); }

    /*!
    Returns a constant iterator to the first entry in the HashTable.
    */
    Iterator<true> begin() const { return Iterator<true>(*this, 0); }

    /*!
    Returns a constant iterator to the last entry in the HashTable.
    */
    Iterator<true> end() const { return Iterator<true>(*this, capacity()); }

    /*!
    Returns a constant iterator to the first entry in the HashTable.
    */
    Iterator<true> cbegin() const { return Iterator<true>(*this, 0); }

    /*!
    Returns a constant iterator to the last entry in the HashTable.
    */
    Iterator<true> cend() const { return Iterator<true>(*this, capacity()); }
};

#endif  // HASH_TABLE_HPP
