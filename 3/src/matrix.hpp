#pragma once

#ifndef MATRIX_H
#define MATRIX_H

#include <boost/serialization/access.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/vector.hpp>

#include "array.hpp"

template <typename T>
class Matrix;

template <typename T>
class Row {
private:
    T *items;

    friend class Matrix<T>;

    Row(T *items) : items(items) {}

public:
    /*!
    Looks up the element at the given `index`.
    */
    T &operator[](size_t index) { return items[index]; }

    /*!
    Looks up the element at the given `index`. Const version.
    */
    const T &operator[](size_t index) const { return items[index]; }
};

template <typename T>
class Matrix {
private:
    using ItemsT = Array<T, true>;
    size_t rows;
    size_t columns;
    ItemsT items;

    static size_t index_of(size_t i, size_t rows, size_t j) {
        return i * rows + j;
    }

public:
    using iterator = typename ItemsT::iterator;
    using const_iterator = typename ItemsT::const_iterator;

    /*!
    Constructs an empty matrix.
    */
    Matrix() : Matrix(0, 0) {}

    /*!
    Constructs a matrix with the given `rows` and `columns`.
    */
    Matrix(size_t rows, size_t columns) : rows(rows), columns(columns), items(rows * columns, rows * columns) {}

    /*!
    Returns the number elements in the matrix.
    */
    size_t size() const { return items.size; }

    /*!
    Returns the number of rows.
    */
    size_t row_count() const { return rows; }

    /*!
    Returns the number of columns.
    */
    size_t column_count() const { return columns; }

    /*!
    Resizes the matrix.
    If the new axes size is smaller than the current size, the elements are truncated.
    If the new axes size is larger than the current size, the elements are filled with default values.
    */
    void resize(size_t rows, size_t columns) {
        ItemsT new_items(rows * columns, rows * columns);

        size_t save_rows = std::min(rows, this->rows);
        size_t save_columns = std::min(columns, this->columns);
        for (size_t i = 0; i < save_rows; ++i) {
            for (size_t j = 0; j < save_columns; ++j) {
                new_items[index_of(i, rows, j)] = items[index_of(i, this->rows, j)];
            }
        }

        items = std::move(new_items);
        this->rows = rows;
        this->columns = columns;
    }

    /*!
    Looks up the row at the given `index`.
    */
    Row<T> operator[](size_t index) {
        return &items[index_of(index, this->rows, 0)];
    }

    /*!
    Looks up the row at the given `index`. Сonst version.
    */
    const Row<T> operator[](size_t index) const {
        return const_cast<Matrix<T>*>(this)->operator[](index);
    }

    /*!
    Get an indices of the item in the matrix by a reference.
    */
    std::pair<std::size_t, std::size_t> indices_of(const T &item) const {
        std::size_t io = items.index_of(item);
        return std::make_pair(io / rows, io % rows);
    }

    /*!
    Returns a predecate iterator. To check for end use regunal end().
    */
    template <typename Pred>
    auto find(Pred pred) { return items.find(pred); }

    /*!
    Returns a predecate iterator. To check for end use regunal end(). Const version.
    */
    template <typename Pred>
    auto find(Pred pred) const { return items.find(pred); }

    /*!
    Returns an iterator to the first element.
    */
    iterator begin() { return items.begin(); }

    /*!
    Returns an iterator to the end.
    */
    iterator end() { return items.end(); }

    /*!
    Returns a constant iterator to the first element.
    */
    const_iterator begin() const { return items.begin(); }

    /*!
    Returns a constant iterator to the end.
    */
    const_iterator end() const { return items.end(); }

    /*!
    Returns a constant iterator to the first element.
    */
    const_iterator cbegin() const { return items.cbegin(); }

    /*!
    Returns a constant iterator to the end.
    */
    const_iterator cend() const { return items.cend(); }

private:
    friend class boost::serialization::access;

    template <class Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar &items;
    }
};

#endif  // MATRIX_H
