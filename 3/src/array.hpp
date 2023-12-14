#pragma once

#ifndef ARRAY_HPP
#define ARRAY_HPP

#include <algorithm>
#include <cstddef>
#include <functional>
#include <utility>
#include <type_traits>
#include <boost/serialization/access.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/vector.hpp>

template <bool is_const, typename T>
using conditional_const_t = std::conditional_t<is_const, std::add_const_t<T>, T>;

// XXX: use std::unique_ptr for data

template <typename T, bool owner = true, bool copy_data_in_copy_ctor = true>
struct Array {
public:
    T *data;               /// The pointer to the array of the elements.
    std::size_t capacity;  /// Number of allocated elements.
    std::size_t size;      /// Number of actually used elements.

    /*!
    Constructs an empty array.
    */
    explicit Array() : Array(0) {}

    /*!
    Constructs an array with the given `capacity`.
    */
    explicit Array(std::size_t capacity) : Array(capacity, 0) {}

    /*!
    Constructs an array with the given `capacity` and `size`.
    */
    explicit Array(std::size_t capacity, std::size_t size)
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
    ~Array() {
        if constexpr (owner) {
            delete[] data;
        }
    }

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
    Get an index of the item in the array by a reference.
    */
    std::size_t index_of(const T &item) const {
        return (&item) - data;
    }

    /*!
    Exchanges the contents of this array and `other` array. Does not create copies.
    */
    void swap(Array &other) {
        std::swap(data, other.data);
        std::swap(capacity, other.capacity);
        std::swap(size, other.size);
    }

    template <bool is_const>
    class Iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = conditional_const_t<is_const, T>;
        using difference_type = std::ptrdiff_t;
        using pointer = value_type *;
        using reference = value_type &;
        using ArrayT = conditional_const_t<is_const, Array<T>>;

    protected:
        ArrayT &array;      /// The reference to the Array that is being iterated.
        std::size_t index;  /// The index of the current item.

    public:
        /*!
        Disallows the default iterator, which consequently iterates nothing.
        */
        Iterator() = delete;

        /*!
        Constructs an iterator for the `array` starting at index 0.
        */
        Iterator(ArrayT &array, std::size_t i) : array(array), index(i) {}

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
            return std::tie(array, index) == std::tie(other.array, other.index);
        }

        /*!
        Checks if the `other` iterator is not equal to this one.
        */
        bool operator!=(const Iterator &other) const { return !(*this == other); }

        /*!
        Advances the iterator to the next item.
        */
        Iterator &operator++() {
            progress();
            return *this;
        }

        /*!
        Returns a copy of this iterator and advances it to the next entry.
        */
        Iterator operator++(int) {
            Iterator result(*this);
            ++(*this);
            return result;
        }

        /*!
        Returns a reference to the current entry.
        */
        reference operator*() const { return array.data[index]; }

    protected:
        void progress() {
            if (index < array.size) {
                ++index;
            }
        }
    };

    using iterator = Iterator<false>;
    using const_iterator = Iterator<true>;

    /*!
    Returns an iterator to the first entry in the Array.
    */
    iterator begin() { return iterator(*this, 0); }

    /*!
    Returns an iterator to the last entry in the Array.
    */
    iterator end() { return iterator(*this, size); }

    /*!
    Returns a constant iterator to the first entry in the Array.
    */
    const_iterator begin() const { return const_iterator(*this, 0); }

    /*!
    Returns a constant iterator to the last entry in the Array.
    */
    const_iterator end() const { return const_iterator(*this, size); }

    /*!
    Returns a constant iterator to the first entry in the Array.
    */
    const_iterator cbegin() const { return const_iterator(*this, 0); }

    /*!
    Returns a constant iterator to the last entry in the Array.
    */
    const_iterator cend() const { return const_iterator(*this, size); }

    template <bool is_const, typename Pred>
    class PredecateIterator : public Iterator<is_const> {
    private:
        Pred pred;

    public:
        /*!
        Disallows the default iterator, which consequently iterates nothing.
        */
        PredecateIterator() = delete;

        /*!
        Constructs an iterator for the `array` starting at index 0.
        */
        PredecateIterator(Iterator<is_const>::ArrayT &array, std::size_t i, Pred pred)
            : Iterator<is_const>(array, i), pred(pred) 
        {
            skip_till_pred_is_true();
        }

        /*!
        Constructs a copy of the other iterator.
        */
        PredecateIterator(const PredecateIterator &) = default;

        /*!
        Copies the other iterator into this.
        */
        PredecateIterator &operator=(const PredecateIterator &) = default;

        /*!
        Advances the iterator to the next item.
        */
        PredecateIterator &operator++() {
            skip_till_pred_is_true();
            this->progress();
            return *this;
        }

        /*!
        Returns a copy of this iterator and advances it to the next predeccate approved entry.
        */
        PredecateIterator operator++(int) {
            Iterator result(*this);
            ++(*this);
            return result;
        }

    private:
        void skip_till_pred_is_true() {
            while (this->index < this->array.size && !pred(this->array[this->index])) {
                ++this->index;
            }
        }
    };

    /*!
    Returns a predecate iterator. To check for end use regunal end().
    */
    template <typename Pred>
    PredecateIterator<false, Pred> find(Pred pred) { return PredecateIterator<false, Pred>(*this, 0, pred); }

    /*!
    Returns a predecate iterator. To check for end use regunal end(). Const version.
    */
    template <typename Pred>
    PredecateIterator<true, Pred> find(Pred pred) const { return PredecateIterator<true, Pred>(*this, 0, pred); }

private:
    friend class boost::serialization::access;

    template <class Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar &capacity;
        ar &size;
        if (Archive::is_loading::value)
        {
            assert(data == nullptr);
            data = new T[capacity];
        }
        for (size_t i = 0; i < capacity; ++i) {
            ar &data[i];
        }
    }
};

namespace std {
    template <typename T>
    void swap(Array<T> &lhs, Array<T> &rhs) {
        lhs.swap(rhs);
    }
}  // namespace std

#endif  // ARRAY_HPP
