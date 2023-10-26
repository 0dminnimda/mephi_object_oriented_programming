
#include "stack_static.h"
    
namespace Lab2{
    Stack::Stack(int count, std::vector<Test> tmp){
        if (count >= m_size){
            throw std::invalid_argument("stakc overflow");
        }
        c_size = count;
        for (int i = 0; i < count; i++){
            arr[i] = tmp[i];
        }
    }
    
    std::vector<Test> Stack::getArr() const{
        std::vector <Test> ans;
        for (int i = 0; i < c_size; i++){
            ans.push_back(arr[i]);
        }
        return ans;
    }

    void Stack::split_stack(){
        int n = 0;
        for (int i = c_size - 1; i >= 0; i--){
            n += arr[i].getLast_num() - arr[i].getFirst_num() + 1; 
        }
        if (n > m_size){
            throw std::invalid_argument("stack overflow");
        }
        Test tmp[n];
        int k = 0;
        int c = c_size;
        for (int i = 0; i < c; i++){
            std::vector<Test> t = (*this).pop().split_test();
            for (size_t j = 0; j < t.size(); j++){
                tmp[k] = t[j];
                k += 1;
            }
        }
        for (int i = 0; i < n; i++){
            (*this) += tmp[i];
        }
    }

    Test Stack::pop(){
        if (c_size == 0){
            throw std::invalid_argument("stack is empty");
        }
        Test ans = arr[c_size - 1];
        c_size -= 1;
        return ans;
    }

    Test Stack::zero_mark(){
        std::vector<Test> tmp;
        int c = c_size;
        int f = c_size;
        for (int i = 0; i < c; i++){
            Test t = (*this).pop();
            tmp.push_back(t);
            if (t.getMark() == 0){
                f = i;
                break;
            }
        }
        for (int i = 0; i < f; i++){
            (*this) += tmp[i];
        }
        if (f == c){
            throw std::invalid_argument("Stack hasn't zero test");
        }
        return tmp[f]; 
    }

    void Stack::delete_test(const Test &test){
        std::vector<Test> tmp;
        int c = c_size;
        int f = c_size;
        for (int i = 0; i < c; i++){
            Test t = (*this).pop();
            tmp.push_back(t);
            if (t <=> test == 0){
                f = i;
                break;
            }
        }
        for (int i = 0; i < f; i++){
            (*this) += tmp[i];
        }
        if (f == c){
            throw std::invalid_argument("Stack hasn't this test");
        }
    }

    void Stack::union_stack(){
        std::unordered_map <std::string, std::vector<int>> mp;
        int c = c_size;
        for (int i = 0; i < c; i++){
            Test test = (*this).pop();
            if (test.getLast_num() > (int) mp[test.getSurname()].size()){
                mp[test.getSurname()].resize(test.getLast_num(), -1);
            }    
            for (int j = test.getFirst_num() - 1; j < test.getLast_num(); j++){
                mp[test.getSurname()][j] = test.getMark();
            }
        }
        c_size = 0;
        std::vector<Test> t;
        for (auto &i: mp){
            int l = -1, r = -1;
            for (size_t j = 0; j < i.second.size(); j++){
                if (i.second[j] != -1){
                    if (l == -1){
                        l = j;
                    }
                    r = j;
                } else if (l != -1){
                    Test tmp(i.first, i.second[l], l + 1, r + 1);
                    t.push_back(tmp);
                    l = -1;
                    r = -1;
                } 
            }
            if (l != -1 && r != -1){
                Test tmp(i.first, i.second[l], l + 1, r + 1);
                t.push_back(tmp);
            }
        }
        for (size_t j = 0; j < t.size(); j++){
            (*this) += t[j];
        }
    }

    void Stack::operator+=(const Test &test){
        if (c_size >= m_size){
            throw std::invalid_argument("stack overflow");
        }
        arr[c_size] = test;
        c_size += 1;
    }

    Stack &Stack::operator=(const Stack &stack){
        c_size = 0;
        for (int i = 0; i < stack.getC_size(); i++){
            (*this) += stack.arr[i];
        }
        return *this;
    }

    Stack &Stack::operator=(Stack &&stack){
        if (this != &stack){
            c_size = 0;
            for (int i = 0; i < stack.c_size; i++){
                (*this) += stack.arr[i];
            }
            stack.c_size = 0;
        }  
        return *this;
    }

    Stack &Stack::operator++(){
        if (c_size == 0){
            throw std::invalid_argument("stack is empty");
        }
        Test tmp = arr[c_size - 1];
        (*this) += tmp;
        return *this;
    }

    Test&Stack::operator[](int i){
        if (i >= c_size || i < 0){
            throw std::invalid_argument("index out of range");
        }
        return arr[i];
    }

    std::ostream &operator<<(std::ostream &c, const Stack &stack){
        if(stack.c_size == 0){
            c << "Stack is empty";
        }else{
            c << "Your stack: "<<std::endl;
            for(int i = stack.c_size - 1; i >= 0; --i){
                c<<"----------"<<i+1<<"----------"<<std::endl;
                c<<stack.arr[i];
            }
        }    
        c << std::endl;
        return c;
    }

    std::istream &operator>>(std::istream &c, Stack &stack){
        if (stack.c_size != 0){
            throw std::invalid_argument("stack isn't empty");
        }
        try{
            int t;
            c>>t;
            if (c.good()){
                for (int i = 0; i < t; i++){
                    Test test;
                    c>>test;
                    stack += test;
                }
            } else{
                throw std::invalid_argument("invalid argument");
            }    
        } catch(...){
            throw;
        }    
        return c;
    }

}