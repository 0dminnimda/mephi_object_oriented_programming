#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "../cocktail.cpp"
#include "doctest.h"


TEST_CASE("sugma") {
    SUBCASE("default ctor") {
        Cocktail cock;
        CHECK(cock == Cocktail());
        CHECK(cock == Cocktail(0, 0));
    }

    SUBCASE("ctor with volume") {
        Cocktail cock(10);
        CHECK(cock == Cocktail(10));
        CHECK(cock == Cocktail(10, 0));
    }

    SUBCASE("ctor with volume and alcohol_fraction") {
        Cocktail cock(10, 0.1);
        CHECK(cock == Cocktail(10, 0.1));
    }

    SUBCASE("ctor with bad volume") { CHECK_THROWS_AS(Cocktail(-1), std::runtime_error); }

    SUBCASE("ctor with bad volume and alcohol_fraction") {
        CHECK_THROWS_AS(Cocktail(-1, 0.1), std::runtime_error);
    }

    SUBCASE("ctor with volume and bad alcohol_fraction") {
        CHECK_THROWS_AS(Cocktail(1, -2), std::runtime_error);
        CHECK_THROWS_AS(Cocktail(1, 1.1), std::runtime_error);
        CHECK_THROWS_AS(Cocktail(1, -0.1), std::runtime_error);
    }

    SUBCASE("equality") {
        SUBCASE("1") {
            Cocktail cock1;
            Cocktail cock2;
            CHECK(cock1 == cock2);
            CHECK(cock1.volume() == cock2.volume());
            CHECK(cock1.alcohol_fraction() == cock2.alcohol_fraction());
        }

        SUBCASE("2") {
            Cocktail cock1(3.14);
            Cocktail cock2(3.14);
            CHECK(cock1 == cock2);
            CHECK(cock1.volume() == cock2.volume());
            CHECK(cock1.alcohol_fraction() == cock2.alcohol_fraction());
        }

        SUBCASE("3") {
            Cocktail cock1(6.9, 0.1);
            Cocktail cock2(6.9, 0.1);
            CHECK(cock1 == cock2);
            CHECK(cock1.volume() == cock2.volume());
            CHECK(cock1.alcohol_fraction() == cock2.alcohol_fraction());
        }
    }

    SUBCASE("inequality") {
        SUBCASE("1") {
            Cocktail cock1(3.14);
            Cocktail cock2(3.1415);
            CHECK(cock1 != cock2);
            CHECK(cock1.volume() != cock2.volume());
            CHECK(cock1.alcohol_fraction() == cock2.alcohol_fraction());
        }

        SUBCASE("2") {
            Cocktail cock1(6.9, 0.1);
            Cocktail cock2(6.9, 0.2);
            CHECK(cock1 != cock2);
            CHECK(cock1.volume() == cock2.volume());
            CHECK(cock1.alcohol_fraction() != cock2.alcohol_fraction());
        }

        SUBCASE("3") {
            Cocktail cock1(6.9, 0.1);
            Cocktail cock2(9.6, 0.2);
            CHECK(cock1 != cock2);
            CHECK(cock1.volume() != cock2.volume());
            CHECK(cock1.alcohol_fraction() != cock2.alcohol_fraction());
        }
    }

    SUBCASE("properties") {
        Cocktail cock;

        cock.volume(10);

        CHECK(cock == Cocktail(10, 0));
        CHECK(cock.volume() == 10);
        CHECK(cock.alcohol_fraction() == 0);

        cock.alcohol_fraction(0.1);

        CHECK(cock == Cocktail(10, 0.1));
        CHECK(cock.volume() == 10);
        CHECK(cock.alcohol_fraction() == (float)0.1);

        cock.volume(69);

        CHECK(cock == Cocktail(69, 0.1));
        CHECK(cock.volume() == 69);
        CHECK(cock.alcohol_fraction() == (float)0.1);

        cock.alcohol_fraction(0.42);

        CHECK(cock == Cocktail(69, 0.42));
        CHECK(cock.volume() == 69);
        CHECK(cock.alcohol_fraction() == (float)0.42);
    }

    SUBCASE("add") {
        CHECK(Cocktail(10, 0.1) + Cocktail(15) == Cocktail(25, 0.04));
        CHECK(Cocktail(15) + Cocktail(10, 0.1) == Cocktail(25, 0.04));
        CHECK(Cocktail() + Cocktail() == Cocktail(0, 0));
        CHECK(Cocktail(0) + Cocktail(0) == Cocktail(0, 0));
        CHECK(Cocktail(0, 0.1) + Cocktail(0, 0.2) == Cocktail(0, 0));
        CHECK(Cocktail(10, 0.1) + Cocktail() == Cocktail(10, 0.1));
        CHECK(Cocktail() + Cocktail(10, 0.1) == Cocktail(10, 0.1));
    }

    SUBCASE("mul") { CHECK(Cocktail(25, 0.04) * 3 == Cocktail(75, 0.04)); }

    SUBCASE("zero volume") {
        CHECK(Cocktail() == Cocktail(0, 0));
        CHECK(Cocktail(0) == Cocktail(0, 0));
        CHECK(Cocktail(0, 0.1) == Cocktail(0, 0));
        CHECK(Cocktail(0, 0.1).alcohol_fraction() == 0);
    }

    SUBCASE("empty") {
        Cocktail cock(10, 0.1);
        CHECK(!cock.is_empty());

        cock.empty();

        CHECK(cock == Cocktail(0, 0));
        CHECK(cock.is_empty());
    }

    SUBCASE("pour") {
        Cocktail cock(10, 0.1);

        Cocktail additional;

        cock.pour(additional, 2);

        CHECK(cock == Cocktail(8, 0.1));
        CHECK(additional == Cocktail(2, 0.1));
    }

    SUBCASE("pour by >>") {
        SUBCASE("basic") {
            Cocktail cock(10, 0.1);

            Cocktail one_more(4, 1);
            Cocktail and_one_more(3, 0.1);

            cock >> one_more >> and_one_more;

            CHECK(cock == Cocktail(8, 0.1));
            CHECK(one_more == Cocktail(5, 0.82));
            CHECK(and_one_more == Cocktail(4, 0.1));
        }

        SUBCASE("pour more than available") {
            Cocktail cock(10, 0.1);

            Cocktail just_the_on_the_bottom(0.2, 0.1);
            just_the_on_the_bottom >> cock;

            CHECK(cock == Cocktail(10.2, 0.1));
            CHECK(just_the_on_the_bottom == Cocktail(0, 0));
        }
    }

    SUBCASE("example") {
        Cocktail cock(10, 0.1);
        Cocktail pure_watah(15);

        Cocktail sum = cock + pure_watah;
        Cocktail big_sum = sum * 3;

        CHECK(cock == Cocktail(10, 0.1));
        CHECK(pure_watah == Cocktail(15));
        CHECK(sum == Cocktail(25, 0.04));
        CHECK(big_sum == Cocktail(75, 0.04));
    }
}
