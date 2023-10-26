#include <iostream>
#include "./TestLib/test.h"



#if defined DYNAMIC
    #include "./StackLib/stack.h"
#else
    #include "./StackStaticLib/stack_static.h"
#endif    
#include "menu.h"
#include "help.h"

using namespace Lab2;
int main(){
    try{
        Stack stack;
        while (true){
            int (*action)(Stack &stack) = menu();
            if (!action){
                break;
            }
            try{
                if (action(stack) != 0){
                    throw std::invalid_argument("poop");
                }
            }catch(const std::exception &msg){
                std::cout << msg.what() << std::endl;
                continue;
            }
            const Stack t(stack);
            Test test;
            t[0] = test;
            std::cout<<t;
        }
    } catch(const std::bad_alloc &ba){
        std::cerr<<"Not enough memory"<<std::endl;
        return 1;    
    } catch(const std::exception &e) {
        std::cerr << e.what()<<std::endl;
        return 1;
    }
    return 0;    
}