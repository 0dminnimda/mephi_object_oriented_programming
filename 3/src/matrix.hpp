#include <vector>

template <typename T>
class Row {
   std::vector<T> items;

public:
   Row(size_t size, T initialValue) : items(size, initialValue) {}
   Row(size_t size) : items(size) {}

   T &operator[](size_t index) { return items[index]; }

   const T &operator[](size_t index) const { return items[index]; }
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
    
    void resize(size_t rows, size_t cols, T initialValue) {
        this->rows.resize(rows, Row<T>(cols, initialValue));
    }
    void resize(size_t rows, size_t cols) {
        this->rows.resize(rows, Row<T>(cols));
    }
    
    Row<T> &operator[](size_t index) { return rows[index]; }
    
    const Row<T> &operator[](size_t index) const { return rows[index]; }
};

