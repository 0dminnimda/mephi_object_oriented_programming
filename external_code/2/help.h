#ifndef HELP_H
#define HELP_H
#include <iostream>
#include <vector>
#include <limits>

#include "./TestLib/test.h"

using namespace Lab2;


template <class T>
T getNum(T min = std::numeric_limits<T>::min(), T max = std::numeric_limits<T>::max()){
    T a;
    while (true){
        std::cin>>a;
        if (std::cin.eof()){
            throw std::runtime_error("Failed to read number: EOF.");
        } else if (std::cin.bad()){
            throw std::runtime_error("Failed to read number.");
        } else if(std::cin.fail() || !std::cin.good()){
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout<<"Error, repeat, please!"<<std::endl;
        } else if (a >= min && a <= max){
            return a;
        } else{
            std::cout<<"Out of range index. Repeat, pls: ";
        }
    }
}

Test inputTest();


#endif