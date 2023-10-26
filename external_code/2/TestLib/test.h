#ifndef TEST_H
#define TEST_H
#include <iostream>
#include <vector>


namespace Lab2{
    //!Класс работы студента
    class Test{
        private:
            //!Фамилия
            std::string surname;
            //!Оценка 0, 2-5
            int mark;
            //!Первая страница
            int first_num;
            //!Последняя страница 
            int last_num;
        public:
            //! Конструктор по умолчанию
            explicit Test(): surname(""), mark(0), first_num(1), last_num(1){};
            //! Конструктор по фамилии
            explicit Test(const std::string &surname);
            //! Конструктор по фамилии, оценки, первой странице, последней странице соответсвенно
            explicit Test(const std::string &surname, int mark, int first_num, int last_num);

            //! Геттер фамилии
            const std::string &getSurname() const {return surname; }
            //! Геттер оценки
            int getMark() const {return mark; }
            //! Геттер первой страницы
            int getFirst_num() const {return first_num; }
            //! Геттер последней страницы
            int getLast_num() const {return last_num; };

            //! Сеттер фамилии
            Test &setSurname(std::string surname);
            //! Сеттер оценки
            Test &setMark(int mark);
            //! Сеттер первой страницы
            Test &setFirst_num(int first_num);
            //! Сеттер послледней страницы
            Test &setLast_num(int last_num);
            
            //! Оператор сложения, возвращает результат
            Test operator+(const Test &test) const;
            //! Функция разделяет на отдельные страницы, поданный в аргументе тест
            std::vector<Test> split_test() const;
            //! Сравнение тестов. Возвращает -1, если первый тест меньше, 0 если равны, 1, если второе больше
            //! Сравнение идет по фамилии, затем по первой странице
            int operator<=>(const Test &test) const;
            //! Отрицание теста: Если тест проверен, но делаем непроверенный, иначе - наоборот, возвращаем измененный тест
            Test operator!();
            //! Копирование теста
            /*Test &operator=(const Test &test);*/

            //! Вывод в поток из аргумента, возвращаем поток
            friend std::ostream &operator<<(std::ostream &c, const Test &test);
            //! Ввод в поток из аргумента, возвращаем поток
            friend std::istream &operator>>(std::istream &c, Test &test);
    };
    
}

#endif