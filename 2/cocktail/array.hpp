#pragma once

#ifndef ARRAY_HPP
#define ARRAY_HPP

#include <algorithm>
#include <cstddef>
#include <functional>
#include <utility>

template <typename T, bool copy_data_in_copy_ctor = true>
struct Array {
public:
    T *data;
    std::size_t capacity;
    std::size_t size;

    Array() : Array(0) {}
    Array(std::size_t capacity) : Array(capacity, 0) {}
    Array(std::size_t capacity, std::size_t size)
        : data(capacity ? new T[capacity] : nullptr), capacity(capacity), size(size) {}

    Array(const Array &other) : Array(other.capacity, other.size) {
        if constexpr (copy_data_in_copy_ctor) {
            std::copy_n(other.data, capacity, data);
        }
    }
    Array(Array &&other) noexcept
        : data(std::exchange(other.data, nullptr)),
          capacity(std::exchange(other.capacity, 0)),
          size(std::exchange(other.size, 0)) {}

    ~Array() { delete[] data; }

    Array &operator=(const Array &other) = default;
    Array &operator=(Array &&other) = default;

    bool operator==(const Array &other) const {
        if (size != other.size) return false;
        if (data == other.data) return true;
        return std::equal(data, data + size, other.data);
    }
    bool operator!=(const Array &other) const { return !(*this == other); }

    const T &operator[](std::size_t index) const { return data[index]; }
    T &operator[](std::size_t index) { return data[index]; }

    void swap(Array &other) {
        std::swap(data, other.data);
        std::swap(capacity, other.capacity);
        std::swap(size, other.size);
    }
};

namespace std {
    template <typename T>
    void swap(Array<T> &lhs, Array<T> &rhs) {
        lhs.swap(rhs);
    }
}  // namespace std

#endif  // ARRAY_HPP
