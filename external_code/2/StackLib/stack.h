#ifndef STACK_H
#define STACK_H

#include <iostream>
#include <vector>
#include <unordered_map>
#include "../TestLib/test.h"

namespace Lab2{
    //! Класс Стопки работ
    class Stack{
        private:
            //! Прибавление массива
            static const int step = 10;
            //! Максимальный размер
            int m_size;
            //! Текущий размер
            int c_size;
            //! Массив работ
            Test *arr;
        public:
            //! Конструктор по умолчании
            Stack(): m_size(10), c_size(0), arr(nullptr){};
            //! Конструктор копирования
            Stack(const Stack& other);
            //! Конструктор по количеству, массива работ, максимальному размеру соответсвенно
            Stack(int count, std::vector<Test> tmp, int m_size);
            //! Конструктор по перемещения
            Stack(Stack&& other);
            //! Деструктор
            ~Stack() {m_size = 0, c_size = 0, delete [] arr; };
            //! Геттер максмимального размера
            int getM_size() const { return m_size; };
            //! Геттер текущего размера
            int getC_size() const { return c_size; };
            //! Геттер массива работ
            Test *getArr() const { return arr; };

            
            
            //! Проверка состояния мссива. Пара: (текущий размер, максимальный рахмер)
            std::pair <int, int> check() const { return std::make_pair(c_size, m_size); };

            //! Разделить каждый тест стопки, на отдельные работы
            void split_stack();
            //! Достать верхний тест
            Test pop();
            //! Достать первый тест без оценки
            Test zero_mark();
            //! Удалить тест в стопке, переданный из аргумента
            void delete_test(const Test &test);
            //! Объединить все тесты в стопке
            void union_stack();

            //! Положить на стопку тест из аргумента
            Stack &operator+=(const Test &test);
            //! Скопировать экземпляр класса
            Stack &operator=(const Stack &stack);
            //! Переместить rvalue
            Stack &operator=(Stack &&stack);
            //! Верхнюю работу скопировать и поместить на верх
            Stack &operator++();
            Stack operator++(int);
            //! Обратиться к стtку по индексу из аргумента
            Test &operator[](int i);
            const Test &operator[](int i) const;
            //! Вывод в выходной поток из аргумента, стэк из аргумента
            friend std::ostream &operator<<(std::ostream &c, const Stack &stack);
            //! Ввод в выходной поток из аргумента, стэк из аргумента
            friend std::istream &operator>>(std::istream &c, Stack &stack);
        


    };
}


#endif