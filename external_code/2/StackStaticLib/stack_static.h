#ifndef STACK_H
#define STACK_H

#include <iostream>
#include <vector>
#include <unordered_map>
#include "../TestLib/test.h"

namespace Lab2{
    class Stack{
        private:
            static const int m_size = 10;
            int c_size;
            Test arr[m_size];
        public:
            Stack(): c_size(0){};  
            Stack(int count, std::vector<Test> tmp);
            ~Stack() {c_size = 0;};

            int getM_size() const { return m_size; };
            int getC_size() const { return c_size; };
            std::vector<Test> getArr() const;

            

            std::pair <int, int> check() const { return std::make_pair(c_size, m_size); };

            void split_stack();
            Test pop();
            Test zero_mark();
            void delete_test(const Test &test);
            void union_stack();


            void operator+=(const Test &test);
            Stack &operator=(const Stack &stack);
            Stack &operator=(Stack &&stack);
            Stack &operator++();
            Test &operator[](int i);
            friend std::ostream &operator<<(std::ostream &c, const Stack &stack);
            friend std::istream &operator>>(std::istream &c, Stack &stack);
        


    };
}






/*Stack &setM_size(int m_size){ this->m_size = m_size; return *this;};
            Stack &setC_size(int c_size) { this->c_size = c_size; return *this; };
            Stack &setArr(Test *arr) { this->arr= arr; return *this; };*/

#endif