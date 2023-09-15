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

template <typename Key, typename Value>
class HashTable {
public:
    class Iterator;

    class Entry {
    private:
        Key key;
        Value value;
        bool busy;

        friend Iterator;
        friend HashTable;

    public:
        Entry() : key(), value(), busy(false) {}

        const Key &first() const { return key; }
        Value &second() { return value; }
        const Value &second() const { return value; }
    };

private:
    Entry *table;
    std::size_t capacity;
    std::size_t size;

    using Self = HashTable<Key, Value>;

public:
    HashTable() : HashTable(0) {}
    HashTable(std::size_t capacity) : capacity(capacity), size(0) { table = new Entry[capacity]; }

    ~HashTable() { delete[] table; }

    bool operator==(const Self &other) const {
        return std::tie(table, capacity, size) == std::tie(other.table, other.capacity, other.size);
    }
    bool operator!=(const Self &other) const { return !(*this == other); }

    void swap(Self &other) {
        std::swap(table, other.table);
        std::swap(capacity, other.capacity);
        std::swap(size, other.size);
    }

    void insert(const Key &key, const Value &value) {
        try_rehash();

        std::size_t index;
        if (find_index(key, index)) {
            table[index].value = value;
            return;
        }

        assert((index < capacity) && "Ur fucked! Rehashing didn't help");

        table[index].key = key;
        table[index].value = value;
        table[index].busy = true;
        ++size;
    }

    bool erase(const Key &key) {
        std::size_t index;
        if (!find_index(key, index)) {
            table[index].busy = false;
            --size;
            return true;
        }
        return false;
    }

    Value &at(const Key &key) {
        std::size_t index;
        if (find_index(key, index)) {
            return table[index].value;
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
            first = !first;

            stream << it.first() << ": " << it.second();
        }
        stream << "}";
        return stream;
    }

private:
    std::size_t key_index(const Key &key) const { return std::hash<Key>{}(key) % capacity; }

    bool find_index(const Key &key, std::size_t &index) const {
        std::size_t i = key_index(key);
        HT_FOR(i, capacity, table[i].busy) {
            if (table[i].key == key) {
                index = i;
                return true;
            }
        }
        index = i;
        return false;
    }

    void try_rehash() {
        std::size_t new_capacity = (size + 1) * 2;
        if (new_capacity <= capacity) return;

        Self new_table(new_capacity);

        for (const auto &it : *this) {
            new_table.insert(it.first(), it.second());
        }

        new_table.swap(*this);
    }

public:
    class Iterator {
    private:
        const HashTable<Key, Value> &table;
        std::size_t index;

    public:
        Iterator(const HashTable<Key, Value> &table, std::size_t i) : table(table), index(i) {
            skip_untill_busy();
        }
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

        const Entry &operator*() const { return table.table[index]; }

    private:
        void progress() {
            if (index < table.capacity) {
                ++index;
            }
        }

        void skip_untill_busy() {
            while (index < table.capacity && !table.table[index].busy) {
                ++index;
            }
        }
    };

    Iterator begin() const { return Iterator(*this, 0); }
    Iterator end() const { return Iterator(*this, capacity); }

    Iterator cbegin() const { return Iterator(*this, 0); }
    Iterator cend() const { return Iterator(*this, capacity); }
};

// template <typename Key, typename Value>
// class HashTable {

//     final int SIZE = 1000000;
//     long[] a = new long[SIZE];
//     int[] ex = new int[SIZE];

//     bool add(long key) {
//         int i = h(key), j = 1;
//         for (; ex[i] != 0; i = (i + 1) % SIZE, j++)
//             if (a[i] == key) return false;
//         ex[i] = j;
//         a[i] = key;
//         return true;
//     }

//     bool contains(long key) {
//         for (int i = h(key); ex[i] != 0; i = (i + 1) % SIZE)
//             if (a[i] == key) return true;
//         return false;
//     }

//     bool remove(long key) {
//         for (int i = h(key); ex[i] != 0; i = (i + 1) % SIZE)
//             if (a[i] == key) {
//                 ex[i] = 0;
//                 compress(i);
//                 return true;
//             }
//         return false;
//     }

//     void compress(int free) {
//         int i = (free + 1) % SIZE, off = 1;
//         for (; ex[i] != 0; i = (i + 1) % SIZE, off++)
//             if (ex[i] > off) {
//                 // move current element
//                 a[free] = a[i];
//                 ex[free] = ex[i] - off;
//                 // mark current slot as free
//                 ex[i] = 0;
//                 off = 0;
//                 free = i;
//             }
//     }
//     int h(long key) { return (int)Math.abs(key % SIZE); }
// }
