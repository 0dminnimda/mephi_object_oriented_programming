#pragma once

#ifndef MATRIX_H
#define MATRIX_H

#include <vector>

template <typename T>
class Row {
    std::vector<T> items;

public:
    using iterator = typename std::vector<T>::iterator;
    using const_iterator = typename std::vector<T>::const_iterator;
    using reverse_iterator = typename std::vector<T>::reverse_iterator;

    Row(size_t size, T initialValue) : items(size, initialValue) {}
    Row(size_t size) : items(size) {}

    size_t size() const { return items.size(); }

    T &operator[](size_t index) { return items[index]; }

    const T &operator[](size_t index) const { return items[index]; }

    iterator begin() { return items.begin(); }
    iterator end() { return items.end(); }
    reverse_iterator rbegin() { return items.rbegin(); }
    reverse_iterator rend() { return items.rend(); }
    const_iterator begin() const { return items.begin(); }
    const_iterator end() const { return items.end(); }
    const_iterator cbegin() const { return items.cbegin(); }
    const_iterator cend() const { return items.cend(); }
};

template <typename T>
class Matrix {
    std::vector<Row<T>> rows;

public:
    using iterator = typename std::vector<Row<T>>::iterator;
    using const_iterator = typename std::vector<Row<T>>::const_iterator;
    using reverse_iterator = typename std::vector<Row<T>>::reverse_iterator;

    Matrix() : Matrix(0, 0) {}
    Matrix(size_t rows, size_t cols, T initialValue) {
        this->rows.resize(rows, Row<T>(cols, initialValue));
    }
    Matrix(size_t rows, size_t cols) { this->rows.resize(rows, Row<T>(cols)); }

    size_t size() const { return rows.size(); }
    size_t row_size() const {
        if (rows.size()) return rows[0].size();
        return 0;
    }

    void resize(size_t rows, size_t cols, T initialValue) {
        this->rows.resize(rows, Row<T>(cols, initialValue));
    }
    void resize(size_t rows, size_t cols) { this->rows.resize(rows, Row<T>(cols)); }

    Row<T> &operator[](size_t index) { return rows[index]; }

    const Row<T> &operator[](size_t index) const { return rows[index]; }

    iterator begin() { return rows.begin(); }
    iterator end() { return rows.end(); }
    reverse_iterator rbegin() { return rows.rbegin(); }
    reverse_iterator rend() { return rows.rend(); }
    const_iterator begin() const { return rows.begin(); }
    const_iterator end() const { return rows.end(); }
    const_iterator cbegin() const { return rows.cbegin(); }
    const_iterator cend() const { return rows.cend(); }
};

#endif // MATRIX_H
