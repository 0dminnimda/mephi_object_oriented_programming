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
    T *data;               /// The pointer to the array of the elements.
    std::size_t capacity;  /// Number of allocated elements.
    std::size_t size;      /// Number of actually used elements.

    /*!
    Constructs an empty array.
    */
    Array() : Array(0) {}

    /*!
    Constructs an array with the given `capacity`.
    */
    Array(std::size_t capacity) : Array(capacity, 0) {}

    /*!
    Constructs an array with the given `capacity` and `size`.
    */
    Array(std::size_t capacity, std::size_t size)
        : data(capacity ? new T[capacity] : nullptr), capacity(capacity), size(size) {}

    /*!
    Constructs a copy of the `other` array.
    */
    Array(const Array &other) : Array(other.capacity, other.size) {
        if constexpr (copy_data_in_copy_ctor) {
            std::copy_n(other.data, capacity, data);
        }
    }

    /*!
    Construct-moves the `other` array.
    */
    Array(Array &&other) noexcept
        : data(std::exchange(other.data, nullptr)),
          capacity(std::exchange(other.capacity, 0)),
          size(std::exchange(other.size, 0)) {}

    /*!
    Destructs the array.
    */
    ~Array() { delete[] data; }

    /*!
    Copies the `other` array into this.
    */
    Array &operator=(const Array &other) {
        if (&other != this) {
            Array temp(other);
            temp.swap(*this);
        }
        return *this;
    }

    /*!
    Moves the `other` array into this.
    */
    Array &operator=(Array &&other) {
        Array temp(std::move(other));
        temp.swap(*this);
        return *this;
    }

    /*!
    Checks if the `other` array is equal to this one.
    */
    bool operator==(const Array &other) const {
        if (size != other.size) return false;
        if (data == other.data) return true;
        return std::equal(data, data + size, other.data);
    }

    /*!
    Checks if the `other` array is not equal to this one.
    */
    bool operator!=(const Array &other) const { return !(*this == other); }

    /*!
    Looks up the element at the given `index`. Returns const reference, thus does not allow to
    modify the array.
    */
    const T &operator[](std::size_t index) const { return data[index]; }

    /*!
    Looks up the element at the given `index`.
    */
    T &operator[](std::size_t index) { return data[index]; }

    /*!
    Exchanges the contents of this array and `other` array. Does not create copies.
    */
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
