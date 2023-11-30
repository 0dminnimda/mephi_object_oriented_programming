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
    T &operator[](size_t index) { return items[index]; }

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

    Matrix() : Matrix(0, 0) {}
    Matrix(size_t rows, size_t columns) : rows(rows), columns(columns), items(rows * columns, rows * columns) {}

    size_t size() const { return items.size; }
    size_t row_count() const { return rows; }
    size_t column_count() const { return columns; }

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

    Row<T> operator[](size_t index) {
        return &items[index_of(index, this->rows, 0)];
    }

    const Row<T> operator[](size_t index) const {
        return const_cast<Matrix<T>*>(this)->operator[](index);
    }

    iterator begin() { return items.begin(); }
    iterator end() { return items.end(); }
    const_iterator begin() const { return items.begin(); }
    const_iterator end() const { return items.end(); }
    const_iterator cbegin() const { return items.cbegin(); }
    const_iterator cend() const { return items.cend(); }

private:
    friend class boost::serialization::access;

    template <class Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar &items;
    }
};

#endif  // MATRIX_H
