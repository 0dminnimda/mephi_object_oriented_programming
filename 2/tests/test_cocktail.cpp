#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "../cocktail/cocktail.hpp"
#include "doctest.h"


TEST_CASE("sugma") {
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

            cock >> one_more >> and_one_more;

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
}
