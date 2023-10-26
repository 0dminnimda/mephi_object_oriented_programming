#define CATCH_CONFIG_MAIN

#include <catch2/catch.hpp>
#include "../TestLib/test.h"
#include "../StackLib/stack.h"

TEST_CASE("TestConstructor") {
    SECTION("DefaultConstructor") {
        Lab2::Test a1;
        REQUIRE(a1.getMark() == 0);
        REQUIRE(a1.getSurname() == "");
        REQUIRE(a1.getFirst_num() == 1);
        REQUIRE(a1.getLast_num() == 1);
    }
    SECTION("InitConstructors") {
        std::string tmp = "strugov";
        Lab2::Test a2(tmp);
        REQUIRE(a2.getSurname() == tmp);
        REQUIRE(a2.getMark() == 0);
        REQUIRE(a2.getFirst_num() == 1);
        REQUIRE(a2.getLast_num() == 1);
        Lab2::Test a3(tmp, 5, 1, 10);
        REQUIRE(a3.getSurname() == tmp);
        REQUIRE(a3.getMark() == 5);
        REQUIRE(a3.getFirst_num() == 1);
        REQUIRE(a3.getLast_num() == 10);
        Lab2::Test a;
        Lab2::Test b("strugov", 5, 5, 6);
        Lab2::Test c("strugov2", 5, 1, 5);
        Lab2::Test d("strugov2", 5, 6, 7);
        Lab2::Test e("strugov", 5, 10, 20);
        a.setSurname("strugov");
        a.setMark(2);
        a.setFirst_num(1);
        a.setLast_num(10);
        REQUIRE(a.getMark() == 2);
        a.setMark(3);
        REQUIRE(a.getMark() == 3);
        a.setMark(4);
        REQUIRE(a.getMark() == 4);
        a.setMark(5);
        REQUIRE_THROWS(b + e);
        REQUIRE(a.getMark() == 5);
        REQUIRE(a.getSurname() == "strugov");
        REQUIRE(a.getFirst_num() == 1);
        REQUIRE(a.getLast_num() == 10);
        REQUIRE_THROWS(a.setMark(-1));
        REQUIRE_THROWS(a.setLast_num(-1));
        REQUIRE_THROWS(a.setFirst_num(-1));
        REQUIRE_THROWS(a.setSurname(""));
        REQUIRE_NOTHROW(a.split_test());
        REQUIRE_NOTHROW(c + d);
        REQUIRE_NOTHROW(d + c);
        REQUIRE_NOTHROW(b <=> c);
        REQUIRE_NOTHROW(c <=> b);
        REQUIRE_NOTHROW(b <=> b);
        REQUIRE_NOTHROW(!b);
        REQUIRE_NOTHROW(std::cout<<a);
        REQUIRE_NOTHROW(std::cin>>a);
        
    }
    SECTION("TestException") {
        REQUIRE_THROWS(Lab2::Test(""));
        REQUIRE_THROWS(Lab2::Test("", 1, 1, 10));
        REQUIRE_THROWS(Lab2::Test("strugov", -1, 1, 10));
        REQUIRE_THROWS(Lab2::Test("strugov", 2, 10, 1));
        REQUIRE_THROWS(Lab2::Test("strugov", 3, -1, 10));

    }
    SECTION("Stack") {
        Lab2::Stack o;
        Lab2::Test tmp("strugov", 5, 1, 10);
        Lab2::Test tmp2("strugov", 0, 1, 10);
        REQUIRE(o.getM_size() == 10);
        REQUIRE(o.getC_size() == 0);
        Lab2::Stack o2(o);
        Lab2::Stack o3(Lab2::Stack s());
        REQUIRE_NOTHROW(o.split_stack());
        REQUIRE_THROWS(o2.pop());
        o2 += tmp;
        REQUIRE_NOTHROW(o2.pop());
        o2 += tmp2;
        REQUIRE_NOTHROW(o2.zero_mark());
        REQUIRE_THROWS(o2.zero_mark());
        REQUIRE_THROWS(o2.delete_test(Lab2::Test("123", 5, 1, 10)));
        o2 += tmp2;
        REQUIRE_NOTHROW(o2.delete_test(tmp2));
        REQUIRE_NOTHROW(o2.union_stack());
        o2 += tmp2;
        REQUIRE_NOTHROW(o2.union_stack());
        Lab2::Stack o4 = o2;
        ++o2;
        o2 += tmp;
        ++o2;
        REQUIRE_NOTHROW(o2[0]);
        o2 += tmp;
        Lab2::Stack o5;
        REQUIRE_NOTHROW(std::cout<<o2);
        REQUIRE_NOTHROW(std::cin>>o5);
        Lab2::Stack t;
        t += tmp;
        t += tmp;
        Lab2::Test k("strugov", 0, 15, 20);
        t += k;
        o4 = o2;
        REQUIRE_NOTHROW(t.split_stack());
        REQUIRE_NOTHROW(t.union_stack());

        std::vector <Lab2::Test> l;
        l.push_back(tmp);
        Lab2::Stack ans(1, l, 10);
        Lab2::Stack stack = (std::move(ans));
        ans = std::move(stack);
        ans++;
    }    
}

TEST_CASE("TestMethods")
{
    SECTION("Setters") {
        Lab2::Test a;
        Lab2::Test b("strugov", 5, 5, 6);
        Lab2::Test c("strugov2", 5, 1, 5);
        Lab2::Test d("strugov2", 5, 6, 7);
        Lab2::Test e("strugov", 5, 10, 20);
        a.setSurname("strugov");
        a.setMark(2);
        a.setFirst_num(1);
        a.setLast_num(10);
        REQUIRE(a.getMark() == 2);
        a.setMark(3);
        REQUIRE(a.getMark() == 3);
        a.setMark(4);
        REQUIRE(a.getMark() == 4);
        a.setMark(5);
        REQUIRE(a.getMark() == 5);
        REQUIRE(a.getSurname() == "strugov");
        REQUIRE(a.getFirst_num() == 1);
        REQUIRE(a.getLast_num() == 10);
        
        
    }    
}
