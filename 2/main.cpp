#include <iostream>
#include <stdexcept>

#include "cocktail.cpp"

int main() {
    Cocktail cock;

    cock.volume(10);
    cock.alcohol_fraction(0.1);

    Cocktail pure_watah(15);

    Cocktail sum = cock + pure_watah;

    Cocktail big_sum = sum * 3;

    std::cout << cock <<  std::endl;
    std::cout << pure_watah <<  std::endl;
    std::cout << sum <<  std::endl;
    std::cout << big_sum <<  std::endl;

    Cocktail additional;
    big_sum.pour(additional, 2);

    Cocktail one_more(5, 0.4);
    Cocktail and_one_more(6, 0.8);
    big_sum >> one_more >> and_one_more;

    std::cout << additional <<  std::endl;
    std::cout << one_more <<  std::endl;
    std::cout << and_one_more <<  std::endl;
    std::cout << big_sum <<  std::endl;

    Cocktail just_the_on_the_bottom(0.2, 0.1);
    just_the_on_the_bottom >> big_sum;

    std::cout << just_the_on_the_bottom <<  std::endl;
    std::cout << big_sum <<  std::endl;

    try {
        Cocktail(-1);
    } catch (const std::runtime_error &err) {
        std::cout << "Successfully errored: " << err.what() << std::endl;
    }

    try {
        Cocktail(1, -2);
    } catch (const std::runtime_error &err) {
        std::cout << "Successfully errored: " << err.what() << std::endl;
    }

    try {
        Cocktail(1, 1.1);
    } catch (const std::runtime_error &err) {
        std::cout << "Successfully errored: " << err.what() << std::endl;
    }

    try {
        Cocktail(1, -0.1);
    } catch (const std::runtime_error &err) {
        std::cout << "Successfully errored: " << err.what() << std::endl;
    }

    Cocktail empty = Cocktail() + Cocktail();
    std::cout << empty <<  std::endl;
    std::cout << cock + Cocktail() <<  std::endl;
    std::cout << Cocktail() + cock <<  std::endl;

    empty.empty();
    std::cout << empty <<  std::endl;

    return 0;
}
