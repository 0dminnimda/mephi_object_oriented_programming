#include <vector>

template <typename T>
class Row {
    std::vector<T> items;

public:
    Row(size_t size, T initialValue) : items(size, initialValue) {}
    Row(size_t size) : items(size) {}

    size_t size() const { return items.size(); }

    T &operator[](size_t index) { return items[index]; }

    const T &operator[](size_t index) const { return items[index]; }

    typename std::vector<T>::iterator begin() { return items.begin(); }
    typename std::vector<T>::iterator end() { return items.end(); }
    typename std::vector<T>::reverse_iterator rbegin() { return items.rbegin(); }
    typename std::vector<T>::reverse_iterator rend() { return items.rend(); }
    typename std::vector<T>::const_iterator cbegin() const { return items.cbegin(); }
    typename std::vector<T>::const_iterator cend() const { return items.cend(); }
};


template <typename T>
class Matrix {
    std::vector<Row<T>> rows;

public:
    Matrix(size_t rows, size_t cols, T initialValue) {
        this->rows.resize(rows, Row<T>(cols, initialValue));
    }
    Matrix(size_t rows, size_t cols) {
        this->rows.resize(rows, Row<T>(cols));
    }

    size_t size() const { return rows.size(); }
    size_t row_size() const {
        if (rows.size()) return rows[0].size();
        return 0;
    }

    void resize(size_t rows, size_t cols, T initialValue) {
        this->rows.resize(rows, Row<T>(cols, initialValue));
    }
    void resize(size_t rows, size_t cols) {
        this->rows.resize(rows, Row<T>(cols));
    }

    Row<T> &operator[](size_t index) { return rows[index]; }

    const Row<T> &operator[](size_t index) const { return rows[index]; }

    typename std::vector<Row<T>>::iterator begin() { return rows.begin(); }
    typename std::vector<Row<T>>::iterator end() { return rows.end(); }
    typename std::vector<Row<T>>::reverse_iterator rbegin() { return rows.rbegin(); }
    typename std::vector<Row<T>>::reverse_iterator rend() { return rows.rend(); }
    typename std::vector<Row<T>>::const_iterator cbegin() const { return rows.cbegin(); }
    typename std::vector<Row<T>>::const_iterator cend() const { return rows.cend(); }
};
