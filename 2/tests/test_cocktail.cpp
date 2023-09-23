#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "../cocktail/cocktail.hpp"
#include "../cocktail/cocktail_map.hpp"
#include "doctest.h"


TEST_CASE("cocktail") {
    SUBCASE("default ctor") {
        Cocktail cock;
        CHECK(cock == Cocktail());
        CHECK(cock == Cocktail("", 0, 0));
    }

    SUBCASE("ctor with volume") {
        Cocktail cock("G", 10);
        CHECK(cock == Cocktail("G", 10));
        CHECK(cock == Cocktail("G", 10, 0));
    }

    SUBCASE("ctor with volume and alcohol_fraction") {
        Cocktail cock("G", 10, 0.1);
        CHECK(cock == Cocktail("G", 10, 0.1));
    }

    SUBCASE("ctor with bad volume") { CHECK_THROWS_AS(Cocktail("G", -1), std::runtime_error); }

    SUBCASE("ctor with bad volume and alcohol_fraction") {
        CHECK_THROWS_AS(Cocktail("G", -1, 0.1), std::runtime_error);
    }

    SUBCASE("ctor with volume and bad alcohol_fraction") {
        CHECK_THROWS_AS(Cocktail("G", 1, -2), std::runtime_error);
        CHECK_THROWS_AS(Cocktail("G", 1, 1.1), std::runtime_error);
        CHECK_THROWS_AS(Cocktail("G", 1, -0.1), std::runtime_error);
    }

    SUBCASE("equality") {
        SUBCASE("1") {
            Cocktail cock1;
            Cocktail cock2;
            CHECK(cock1 == cock2);
            CHECK(cock1.name() == cock2.name());
            CHECK(cock1.volume() == cock2.volume());
            CHECK(cock1.alcohol_fraction() == cock2.alcohol_fraction());
        }

        SUBCASE("2") {
            Cocktail cock1("G", 3.14);
            Cocktail cock2("G", 3.14);
            CHECK(cock1 == cock2);
            CHECK(cock1.name() == cock2.name());
            CHECK(cock1.volume() == cock2.volume());
            CHECK(cock1.alcohol_fraction() == cock2.alcohol_fraction());
        }

        SUBCASE("3") {
            Cocktail cock1("G", 6.9, 0.1);
            Cocktail cock2("G", 6.9, 0.1);
            CHECK(cock1 == cock2);
            CHECK(cock1.name() == cock2.name());
            CHECK(cock1.volume() == cock2.volume());
            CHECK(cock1.alcohol_fraction() == cock2.alcohol_fraction());
        }
    }

    SUBCASE("inequality") {
        SUBCASE("1") {
            Cocktail cock1("G", 3.14);
            Cocktail cock2("G1", 3.14);
            CHECK(cock1 != cock2);
            CHECK(cock1.name() != cock2.name());
            CHECK(cock1.volume() == cock2.volume());
            CHECK(cock1.alcohol_fraction() == cock2.alcohol_fraction());
        }

        SUBCASE("2") {
            Cocktail cock1("G", 3.14);
            Cocktail cock2("G", 3.1415);
            CHECK(cock1 != cock2);
            CHECK(cock1.name() == cock2.name());
            CHECK(cock1.volume() != cock2.volume());
            CHECK(cock1.alcohol_fraction() == cock2.alcohol_fraction());
        }

        SUBCASE("3") {
            Cocktail cock1("G", 6.9, 0.1);
            Cocktail cock2("G", 6.9, 0.2);
            CHECK(cock1 != cock2);
            CHECK(cock1.name() == cock2.name());
            CHECK(cock1.volume() == cock2.volume());
            CHECK(cock1.alcohol_fraction() != cock2.alcohol_fraction());
        }

        SUBCASE("4") {
            Cocktail cock1("G", 6.9, 0.1);
            Cocktail cock2("G1", 9.6, 0.2);
            CHECK(cock1 != cock2);
            CHECK(cock1.name() != cock2.name());
            CHECK(cock1.volume() != cock2.volume());
            CHECK(cock1.alcohol_fraction() != cock2.alcohol_fraction());
        }
    }

    SUBCASE("properties") {
        Cocktail cock;

        cock.name("G");

        CHECK(cock == Cocktail("G", 0, 0));
        CHECK(cock.name() == "G");
        CHECK(cock.volume() == 0);
        CHECK(cock.alcohol_fraction() == 0);

        cock.volume(10);

        CHECK(cock == Cocktail("G", 10, 0));
        CHECK(cock.name() == "G");
        CHECK(cock.volume() == 10);
        CHECK(cock.alcohol_fraction() == 0);

        cock.alcohol_fraction(0.1);

        CHECK(cock == Cocktail("G", 10, 0.1));
        CHECK(cock.name() == "G");
        CHECK(cock.volume() == 10);
        CHECK(cock.alcohol_fraction() == (float)0.1);

        cock.name("GG");

        CHECK(cock == Cocktail("GG", 10, 0.1));
        CHECK(cock.name() == "GG");
        CHECK(cock.volume() == 10);
        CHECK(cock.alcohol_fraction() == (float)0.1);

        cock.volume(69);

        CHECK(cock == Cocktail("GG", 69, 0.1));
        CHECK(cock.name() == "GG");
        CHECK(cock.volume() == 69);
        CHECK(cock.alcohol_fraction() == (float)0.1);

        cock.alcohol_fraction(0.42);

        CHECK(cock == Cocktail("GG", 69, 0.42));
        CHECK(cock.name() == "GG");
        CHECK(cock.volume() == 69);
        CHECK(cock.alcohol_fraction() == (float)0.42);
    }

    SUBCASE("add") {
        CHECK(Cocktail("GT", 10, 0.1) + Cocktail("G", 15) == Cocktail("GTG", 25, 0.04));
        CHECK(Cocktail("G", 15) + Cocktail("GT", 10, 0.1) == Cocktail("GGT", 25, 0.04));
        CHECK(Cocktail() + Cocktail() == Cocktail("", 0, 0));
        CHECK(Cocktail("A", 0) + Cocktail("B", 0) == Cocktail("AB", 0, 0));
        CHECK(Cocktail("A", 0, 0.1) + Cocktail("B", 0, 0.2) == Cocktail("AB", 0, 0));
        CHECK(Cocktail("H", 10, 0.1) + Cocktail() == Cocktail("H", 10, 0.1));
        CHECK(Cocktail() + Cocktail("H", 10, 0.1) == Cocktail("H", 10, 0.1));
    }

    SUBCASE("mul") { CHECK(Cocktail("R", 25, 0.04) * 3 == Cocktail("R", 75, 0.04)); }

    SUBCASE("zero volume") {
        CHECK(Cocktail() == Cocktail("", 0, 0));
        CHECK(Cocktail("", 0) == Cocktail("", 0, 0));
        CHECK(Cocktail("", 0, 0.1) == Cocktail("", 0, 0));
        CHECK(Cocktail("", 0, 0.1).alcohol_fraction() == 0);
    }

    SUBCASE("empty") {
        Cocktail cock("H", 10, 0.1);
        CHECK(!cock.is_empty());

        cock.empty();

        CHECK(cock == Cocktail("H", 0, 0));
        CHECK(cock.is_empty());
    }

    SUBCASE("pour") {
        Cocktail cock("Y", 10, 0.1);

        Cocktail additional("O", 0);

        cock.pour(additional, 2);

        CHECK(cock == Cocktail("Y", 8, 0.1));
        CHECK(additional == Cocktail("OY", 2, 0.1));
    }

    SUBCASE("pour by >>") {
        SUBCASE("basic") {
            Cocktail cock("E", 10, 0.1);

            Cocktail one_more("G", 4, 1);
            Cocktail and_one_more("D", 3, 0.1);

            cock >> one_more;
            and_one_more << cock;

            CHECK(cock == Cocktail("E", 8, 0.1));
            CHECK(one_more == Cocktail("GE", 5, 0.82));
            CHECK(and_one_more == Cocktail("DE", 4, 0.1));
        }

        SUBCASE("pour more than available") {
            Cocktail cock("E", 10, 0.1);

            Cocktail just_the_on_the_bottom("P", 0.2, 0.1);
            just_the_on_the_bottom >> cock;

            CHECK(cock == Cocktail("EP", 10.2, 0.1));
            CHECK(just_the_on_the_bottom == Cocktail("P", 0, 0));
        }
    }

    SUBCASE("example") {
        Cocktail cock("O", 10, 0.1);
        Cocktail pure_watah("Y", 15);

        Cocktail sum = cock + pure_watah;
        Cocktail big_sum = sum * 3;

        CHECK(cock == Cocktail("O", 10, 0.1));
        CHECK(pure_watah == Cocktail("Y", 15));
        CHECK(sum == Cocktail("OY", 25, 0.04));
        CHECK(big_sum == Cocktail("OY", 75, 0.04));
    }

    SUBCASE("mix") {
        Cocktail a("Vo", 10, 0.2);
        Cocktail b("Ar", 5, 0.8);
        Cocktail result;

        bool ok = Cocktail::mix_for_alcohol_fraction(a, b, result, 0.5, 5);

        CHECK(ok);
        CHECK(result == Cocktail("VoAr", 5, 0.5));
    }

    SUBCASE("mix reverse order") {
        Cocktail a("Vo", 10, 0.2);
        Cocktail b("Ar", 5, 0.8);
        Cocktail result;

        bool ok = Cocktail::mix_for_alcohol_fraction(b, a, result, 0.5, 5);

        CHECK(ok);
        CHECK(result == Cocktail("VoAr", 5, 0.5));
    }

    SUBCASE("mix too much") {
        Cocktail a("Vo", 10, 0.2);
        Cocktail b("Ar", 5, 0.8);
        Cocktail result;

        bool ok = Cocktail::mix_for_alcohol_fraction(b, a, result, 0.5, 15);

        CHECK(!ok);
        CHECK(result == Cocktail("VoAr", 10, 0.5));
    }

    SUBCASE("input") {
        std::istringstream stream("Aboba 10 0.4");

        Cocktail cock;

        stream >> cock;

        CHECK(!stream.fail());
        CHECK(cock == Cocktail("Aboba", 10, 0.4));
    }

    SUBCASE("bad input 1") {
        std::istringstream stream("Aboba 10");

        Cocktail cock;

        stream >> cock;

        CHECK(stream.fail());
        CHECK(cock == Cocktail());
    }

    SUBCASE("bad input 2") {
        std::istringstream stream("Aboba");

        Cocktail cock;

        stream >> cock;

        CHECK(stream.fail());
        CHECK(cock == Cocktail());
    }

    SUBCASE("bad input 3") {
        std::istringstream stream("");

        Cocktail cock;

        stream >> cock;

        CHECK(stream.fail());
        CHECK(cock == Cocktail());
    }

    SUBCASE("output") {
        std::ostringstream stream;

        Cocktail cock("Aboba", 10, 0.4);

        stream << cock;

        CHECK(stream);
        CHECK(stream.str() == "Cocktail(\"Aboba\", 10.000000, 0.400000)");
    }
}


// TEST_CASE("hash table") {
//     SUBCASE("default ctor") {
//         HashTable<std::string, Cocktail> map;
//         std::cout << map << std::endl;

//         {
//             Cocktail cock("Gi", 10);
//             map.insert(cock.name(), cock);
//             std::cout << map.at(cock.name()) << std::endl;
//         }

//         std::cout << map << std::endl;

//         {
//             Cocktail cock("dfgd", 23, 0.1);
//             map.insert(cock.name(), cock);
//             std::cout << map.at(cock.name()) << std::endl;
//         }

//         std::cout << map << std::endl;

//         {
//             Cocktail cock("jghjg", 23, 0.1);
//             map.insert(cock.name(), cock);
//             std::cout << map.at(cock.name()) << std::endl;
//         }

//         std::cout << map << std::endl;

//         {
//             if (map.erase("jghjg")) {
//                 std::cout << "erased" << std::endl;
//             } else {
//                 std::cout << "not erased" << std::endl;
//             }
//         }

//         std::cout << map << std::endl;

//         {
//             if (map.erase("nothing")) {
//                 std::cout << "erased" << std::endl;
//             } else {
//                 std::cout << "not erased" << std::endl;
//             }
//         }

//         std::cout << map << std::endl;
//     }
// }


TEST_CASE("cocktail map") {
    SUBCASE("default ctor") {
        CocktailMap map;
        CHECK(map.size() == 0);
        CHECK(map.capacity() >= 0);
    }

    SUBCASE("move ctor") {
        CocktailMap map(2);
        CHECK(map.size() == 0);
        CHECK(map.capacity() == 2);

        CocktailMap map2(std::move(map));
        CHECK(map.size() == 0);
        CHECK(map.capacity() == 0);
        CHECK(map2.size() == 0);
        CHECK(map2.capacity() == 2);
    }

    SUBCASE("array ctor") {
        Cocktail cocks[] = {Cocktail("a", 10), Cocktail("b", 18), Cocktail("c", 15)};
        CocktailMap map(cocks, sizeof(cocks) / sizeof(cocks[0]));
        CHECK(map.size() == 3);
        CHECK(map.capacity() >= 3);
    }

    SUBCASE("equal") {
        CocktailMap map;
        map += Cocktail("a", 10);
        map += Cocktail("b", 18);
        map += Cocktail("c", 15);

        CocktailMap map2;
        map2 += Cocktail("a", 10);
        map2 += Cocktail("b", 18);
        map2 += Cocktail("c", 15);

        CHECK(map == map2);
    }

    SUBCASE("equal to array") {
        Cocktail cocks[] = {Cocktail("a", 10), Cocktail("b", 18), Cocktail("c", 15)};
        CocktailMap map(cocks, sizeof(cocks) / sizeof(cocks[0]));

        CocktailMap map2;
        map2 += Cocktail("a", 10);
        map2 += Cocktail("b", 18);
        map2 += Cocktail("c", 15);

        CHECK(map == map2);
    }

    SUBCASE("equal mixed order") {
        CocktailMap map(5);
        map += Cocktail("a", 10);
        map += Cocktail("b", 18);
        map += Cocktail("c", 15);
        map += Cocktail("d", 34);
        map += Cocktail("e", 4);

        CocktailMap map2(6);
        map2 += Cocktail("d", 34);
        map2 += Cocktail("c", 15);
        map2 += Cocktail("a", 10);
        map2 += Cocktail("e", 4);
        map2 += Cocktail("b", 18);

        CHECK(map == map2);
    }

    SUBCASE("unequal 1") {
        CocktailMap map;
        map += Cocktail("a", 10);
        map += Cocktail("b", 18);
        map += Cocktail("c", 15);

        CocktailMap map2;
        map2 += Cocktail("a", 100);
        map2 += Cocktail("b", 18);
        map2 += Cocktail("c", 15);

        CHECK(map != map2);
    }

    SUBCASE("unequal 2") {
        CocktailMap map;
        map += Cocktail("a", 10);
        map += Cocktail("b", 18);
        map += Cocktail("c", 15);

        CocktailMap map2;
        map2 += Cocktail("aa", 10);
        map2 += Cocktail("b", 18);
        map2 += Cocktail("c", 15);

        CHECK(map != map2);
    }

    SUBCASE("copy operator") {
        CocktailMap map(2);
        CHECK(map.size() == 0);
        CHECK(map.capacity() == 2);

        CocktailMap map2 = map;
        CHECK(map.size() == 0);
        CHECK(map.capacity() == 2);
        CHECK(map2.size() == 0);
        CHECK(map2.capacity() == 2);
    }

    SUBCASE("move operator") {
        CocktailMap map(2);
        CHECK(map.size() == 0);
        CHECK(map.capacity() == 2);

        CocktailMap map2 = std::move(map);
        CHECK(map.size() == 0);
        CHECK(map.capacity() == 0);
        CHECK(map2.size() == 0);
        CHECK(map2.capacity() == 2);
    }

    SUBCASE("add and lookup") {
        CocktailMap map;

        map += Cocktail("Vo", 10, 0.2);
        CHECK(map.size() == 1);
        CHECK(map.capacity() >= 1);
        CHECK(map["Vo"] == Cocktail("Vo", 10, 0.2));

        map += Cocktail("Ar", 5, 0.8);
        CHECK(map.size() == 2);
        CHECK(map.capacity() >= 2);
        CHECK(map["Vo"] == Cocktail("Vo", 10, 0.2));
        CHECK(map["Ar"] == Cocktail("Ar", 5, 0.8));
    }

    SUBCASE("quartile") {
        CocktailMap map;

        map += Cocktail("Vo", 10, 0.2);
        map += Cocktail("Ar", 5, 0.8);

        CHECK(map.volume_with_alcohol_fraction_in_quartile(Quartile::FIRST) == 10);
        CHECK(map.volume_with_alcohol_fraction_in_quartile(Quartile::SECOND) == 0);
        CHECK(map.volume_with_alcohol_fraction_in_quartile(Quartile::THIRD) == 0);
        CHECK(map.volume_with_alcohol_fraction_in_quartile(Quartile::FOURTH) == 5);
    }

    SUBCASE("rename") {
        CocktailMap map;

        map += Cocktail("Vo", 10, 0.2);
        map += Cocktail("Ar", 5, 0.8);

        map.rename("Ar", "Og");

        CHECK(map.size() == 2);
        CHECK(map.capacity() >= 2);
        CHECK(map["Vo"] == Cocktail("Vo", 10, 0.2));
        CHECK(map["Og"] == Cocktail("Og", 5, 0.8));
    }

    SUBCASE("mix") {
        CocktailMap map;

        map += Cocktail("Vo", 10, 0.2);
        map += Cocktail("Ar", 5, 0.8);

        Cocktail result1 = map.mix_for_alcohol_fraction(0.6);

        CHECK(map.size() == 2);
        CHECK(map.capacity() >= 2);
        CHECK(result1 == Cocktail("VoAr", 5, 0.6));

        Cocktail result2 = map.mix_for_alcohol_fraction(0.3, 1);

        CHECK(map.size() == 2);
        CHECK(map.capacity() >= 2);
        CHECK(result2 == Cocktail("VoAr", 1, 0.3));
    }

    SUBCASE("input") {
        std::istringstream stream("2 Vodka 10 0.6 Beer 3 0.4");

        CocktailMap map;

        stream >> map;

        CocktailMap map2;

        map2 += Cocktail("Vodka", 10, 0.6);
        map2 += Cocktail("Beer", 3, 0.4);

        CHECK(!stream.fail());
        CHECK(map == map2);
    }

    SUBCASE("bad input 1") {
        std::istringstream stream("2 Vodka 10 0.6 Beer 3");

        CocktailMap map;

        stream >> map;

        CocktailMap map2;

        CHECK(stream.fail());
        CHECK(map == map2);
    }

    SUBCASE("bad input 2") {
        std::istringstream stream("2 Vodka 10 0.6 Beer");

        CocktailMap map;

        stream >> map;

        CocktailMap map2;

        CHECK(stream.fail());
        CHECK(map == map2);
    }

    SUBCASE("bad input 3") {
        std::istringstream stream("2 Vodka 10 0.6");

        CocktailMap map;

        stream >> map;

        CocktailMap map2;

        CHECK(stream.fail());
        CHECK(map == map2);
    }

    SUBCASE("output") {
        std::ostringstream stream;

        CocktailMap map;

        map += Cocktail("Vo", 10, 0.2);

        stream << map;

        CHECK(stream);
        CHECK(stream.str() == "{Cocktail(\"Vo\", 10.000000, 0.200000)}");
    }

    SUBCASE("output several") {
        std::ostringstream stream;

        CocktailMap map;

        map += Cocktail("Vo", 10, 0.2);
        // hack for testing, otherwise the ordering is STL implementation detail
        map.insert("Ar", Cocktail("Vo", 10, 0.2));

        stream << map;

        CHECK(stream);
        CHECK(stream.str() == "{Cocktail(\"Vo\", 10.000000, 0.200000), Cocktail(\"Vo\", 10.000000, 0.200000)}");
    }
}
