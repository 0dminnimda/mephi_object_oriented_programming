#include "menu.h"


int (*menu())(Stack &stack){
    try{
        int (*foo[])(Stack &stack) = {enter_stack, show_stack, show_info, add_test, delete_test, link_tests, split_test, split_stack, pop_stack, union_stack, zero_pop, nullptr};
        std::cout<<"Menu:"<<std::endl;
        std::string msg[]{"Enter stack.", "Show stack", "Show info", "Add test", "Delete test", "Link tests", "Split test", "Split stack", "Stack's pop", "Union tests", "Zero pop", "Quit"};
        for (size_t i = 0; i < sizeof(msg) / sizeof(msg[0]); i++){
            std::cout<<i + 1<<". "<<msg[i]<<std::endl;
        }
        int a = getNum<int>(1, 12);
        return foo[a - 1];
    } catch(...){
        throw;
    }    
}

int enter_stack(Stack &stack){
    
    try{
        if (stack.getC_size() != 0){
            throw std::invalid_argument("stack isn't empty");
        }
        int c_size;
        std::cout<<"Enter size stack: ";
        c_size = getNum<int>(0, 10);
        for (int i = 0; i < c_size; i++){
            stack += inputTest();
            std::cout<<std::endl;
        }
    } catch(...){
        throw;
    }
    return 0;    
}

int show_stack(Stack &stack){
    std::cout<<stack;
    return 0;  
}

int show_info(Stack &stack){
    std::unordered_map <std::string, int> mp;
    std::cout<<std::endl;
    std::cout<<"C_size: "<<stack.getC_size()<<std::endl;
    std::cout<<"M_size: "<<stack.getM_size()<<std::endl;
    for (int i = 0; i < stack.getC_size(); i++){
        if (mp.count(stack.getArr()[i].getSurname()) == 0){
            mp[stack.getArr()[i].getSurname()] = 1;
        } else{
            mp[stack.getArr()[i].getSurname()] += 1;
        }
    }
    std::cout<<"Autors and counts: "<<std::endl;
    for (auto &i: mp){
        std::cout<<i.first<<": "<<i.second<<std::endl;
    }
    std::cout<<std::endl;
    return 0;  
}

int add_test(Stack &stack){
    try{
        std::cout<<std::endl;
        Test push = inputTest();
        stack += push;
    } catch(...){
        throw;
    }
    return 0;  
}

int delete_test(Stack &stack){
    try{
        if (stack.getC_size() == 0){
            throw std::invalid_argument("Stack is empty");
        }
        int ind;
        show_stack(stack);
        std::cout<<"Choice test: ";
        ind = getNum<int>(1, stack.getC_size());
        Test first = stack.getArr()[ind-1];
        stack.delete_test(first);
    } catch (...){
        throw;
    }
    return 0;
}

int link_tests(Stack &stack){
    try{
        int f, s;
        show_stack(stack);
        std::cout<<"Choice tests: ";
        f = getNum<int>(1, stack.getC_size());
        s = getNum<int>(1, stack.getC_size());
        Test first = stack.getArr()[s-1];
        Test second = stack.getArr()[f-1];
        std::cout<<(first + second);
        stack.delete_test(first);
        stack.delete_test(second);
        stack += (first + second);
    } catch (...){
        throw;
    }
    return 0;
}

int split_test(Stack &stack){
    try{
        int ind;
        show_stack(stack);
        std::cout<<"Choice test: ";
        ind = getNum<int>(1, stack.getC_size());
        Test first = stack.getArr()[ind-1];
        std::vector <Test> tmp = first.split_test();
        stack.delete_test(first);
        for (size_t i = 0; i < tmp.size(); i++){
            stack += tmp[i];
        }
    } catch(...){
        throw;
    }
    return 0;
}

int split_stack(Stack &stack){
    try{
        stack.split_stack();
    } catch(...){
        throw;
    }    
    return 0;
}

int pop_stack(Stack &stack){
    try{
        Test ans = stack.pop();
        std::cout<<std::endl;
        std::cout<<"Pop test: "<<std::endl;
        std::cout<<ans;
    } catch (...){
        throw;
    }
    return 0;
}

int union_stack(Stack &stack){
    stack.union_stack();
    return 0;
}

int zero_pop(Stack &stack){
    try{
        Test ans = stack.zero_mark();
        std::cout<<std::endl;
        std::cout<<"Zero test: "<<std::endl;
        std::cout<<ans;
    } catch (...){
        throw;
    }
    return 0;
}
