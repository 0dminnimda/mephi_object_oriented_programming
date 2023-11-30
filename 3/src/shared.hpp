#include <exception>
#include <iostream>

#define TRY_CATCH_ALL(code)                                    \
    {                                                          \
        try {                                                  \
            code;                                              \
        } catch (const std::exception &e) {                    \
            std::cout << "Error: " << e.what() << std::endl;   \
        } catch (...) {                                        \
            std::cout << "Unknwon error occured" << std::endl; \
        }                                                      \
    }
