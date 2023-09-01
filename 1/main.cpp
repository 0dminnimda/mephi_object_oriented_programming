#include <iostream>
#include <cstddef>
#include <vector>
#include <unordered_map>
#include <optional>
#include <functional>

template < class T >
T getNum(std::istream& is, T min = std::numeric_limits<T>::min(), T max = std::numeric_limits<T>::max()) {
    T a;
    while (true) {
        is >> a;
        if (is.eof())
            throw std::runtime_error("Failed to read number: EOF");
        else if (is.bad())
            throw std::runtime_error(std::string("Failed to read number: ") + strerror(errno));
        else if (is.fail()) {
            is.clear();
            is.ignore(std::numeric_limits < std::streamsize > ::max(), '\n');
            std::cout << "You are wrong; repeat please!" << std::endl;
        }
        else if (a >= min && a <= max)
            return a;
    }
}

template <typename T>
class SparseMatrix {
private:
    using row_type = std::unordered_map<std::size_t, T>;
    using matrix_type = std::unordered_map<std::size_t, row_type>;
    matrix_type matrix;
    std::size_t rows;
    std::size_t cols;

public:
    SparseMatrix(std::size_t rows, std::size_t cols) : rows(rows), cols(cols) {}
    SparseMatrix(const SparseMatrix &) = default;
    SparseMatrix &operator=(const SparseMatrix &) = default;

    void set_value(std::size_t row, std::size_t col, T value) {
        if (row < 0 || row >= rows || col < 0 || col >= cols) {
            throw std::out_of_range("Invalid index");
        }
        if (value != 0) matrix[row][col] = value;
    }

    std::optional<T> get_value(std::size_t row, std::size_t col) const {
        if (row < 0 || row >= rows || col < 0 || col >= cols) {
            throw std::out_of_range("Invalid index");
        }

        auto outer = matrix.find(row);
        if (outer == matrix.end()) {
            return {};
        }

        auto inner = outer->second.find(col);
        if (inner == outer->second.end()) {
            return {};
        }

        return inner->second;
    }

    void filter_row(std::function<bool(T)> pred, std::size_t i) {
        if (i < 0 || i >= rows) {
            throw std::out_of_range("Invalid index");
        }

        auto outer = matrix.find(i);
        if (outer == matrix.end()) {
            return;
        }

        auto &row_pair = *outer;
        for (auto it = row_pair.second.begin(); it != row_pair.second.end();) {
            if (!pred(it->second)) {
                it = row_pair.second.erase(it);
            } else {
                ++it;
            }
        }
    }

    void sort_rows() {
        using elem_type = std::pair<std::size_t, std::size_t>;
        std::vector<elem_type> len_ids;
        for (std::size_t i = 0; i < rows; ++i) {
            auto outer = matrix.find(i);
            if (outer == matrix.end()) {
                len_ids.push_back(std::make_pair(0, i));
            } else {
                auto &row_pair = *outer;
                len_ids.push_back(std::make_pair(row_pair.second.size(), i));
            }
        }

        std::sort(len_ids.begin(), len_ids.end(), [](const elem_type& a, const elem_type& b) -> bool
        {
            return a.first < b.first;
        });

        matrix_type result;
        result.reserve(matrix.size());
        for (std::size_t i = 0; i < rows; ++i) {
            std::size_t new_i = len_ids[i].second;
            auto outer = matrix.find(new_i);
            if (outer != matrix.end()) {
                auto &row_pair = *outer;
                result[i] = row_pair.second;
            }
        }

        matrix = result;
    }

    friend std::ostream& operator<<(std::ostream& os, const SparseMatrix<T>& mat) {
        for (std::size_t row = 0; row < mat.rows; ++row) {
            for (std::size_t col = 0; col < mat.cols; ++col) {
                if (auto value = mat.get_value(row, col)) {
                    os << *value << " ";
                } else {
                    os << "0 ";
                }
            }
            os << std::endl;
        }
        return os;
    }

    friend std::istream& operator>>(std::istream& is, SparseMatrix<T>& mat) {
        for (std::size_t row = 0; row < mat.rows; ++row) {
            for (std::size_t col = 0; col < mat.cols; ++col) {
                T value = getNum<T>(is);
                mat.set_value(row, col, value);
            }
        }
        return is;
    }
};

bool is_greater_than_zero(int value) {
    return value > 0;
}

void sub_main() {
    std::size_t rows = 3;
    SparseMatrix<int> matrix(rows, 3);

    std::cin >> matrix;

    std::cout << "Source matrix" << std::endl << matrix;

    SparseMatrix<int> filtered = matrix;

    std::cout << "Input row index that will be filtered " << std::flush;
    std::size_t i = getNum<std::size_t>(std::cin, 0, rows - 1);
    filtered.filter_row(is_greater_than_zero, i);

    std::cout << "Filtered matrix" << std::endl << filtered;

    filtered.sort_rows();

    std::cout << "Result matrix" << std::endl << filtered;
}

int main() {
    int return_code = 1;
    try {
        sub_main();
        return_code = 0;
    } catch (const std::exception &exc) {
        std::cout << exc.what() << std::endl;
    } catch (...) {
        std::cout << "Unkown error" << std::endl;
    }
    return return_code;
}