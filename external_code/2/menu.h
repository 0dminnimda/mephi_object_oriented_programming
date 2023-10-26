#ifndef MENU_H
#define MENU_H
#include <iostream>
#include "./TestLib/test.h"
#if defined DYNAMIC
    #include "./StackLib/stack.h"
#else
    #include "./StackStaticLib/stack_static.h"
#endif    
#include "help.h"
#include "unordered_map"

int (*menu())(Stack &stack);
int enter_stack(Stack &stack);
int show_stack(Stack &stack);
int show_info(Stack &stack);
int add_test(Stack &stack);
int delete_test(Stack &stack);
int link_tests(Stack &stack);
int split_test(Stack &stack);
int split_stack(Stack &stack);
int pop_stack(Stack &stack);
int union_stack(Stack &stack);
int zero_pop(Stack &stack);

#endif