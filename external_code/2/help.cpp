#include "help.h"


Test inputTest(){
    try{
        std::string s;
        int mark, first_num, last_num;
        std::cout<<"Enter surname: ";
        std::cin>>s;
        std::cout<<"Enter mark: ";
        mark = getNum<int>();
        std::cout<<"Enter first page: ";
        first_num = getNum<int>(1);
        std::cout<<"Enter last page: ";
        last_num = getNum<int>(first_num);
        Test res(s, mark, first_num, last_num);
        return res;
    } catch(...){
        throw;
    }    
}

