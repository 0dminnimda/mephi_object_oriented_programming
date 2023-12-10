#include <stdexcept>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

TEST_CASE("suit") {
    SUBCASE("hello world") {
        CHECK(true);
        CHECK(false);
    }
}
