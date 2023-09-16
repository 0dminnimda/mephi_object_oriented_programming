#include <cassert>
#include <cstddef>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <tuple>

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
    HashTable(std::size_t capacity) : capacity_(capacity), size_(0) {
        entries_ = new Entry[capacity];
    }
    HashTable(const Self &other) = default;
    HashTable(Self &&other) = default;

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
        using TableT = conditional_const_t<is_const, HashTable<Key, Value>>;
        TableT &table;
        std::size_t index;

    public:
        Iterator(TableT &table, std::size_t i) : table(table), index(i) { skip_untill_busy(); }
        Iterator(const Iterator &) = default;

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
            progress();
            skip_untill_busy();
            return result;
        }

        conditional_const_t<is_const, Entry> &operator*() const { return table.entries_[index]; }

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

