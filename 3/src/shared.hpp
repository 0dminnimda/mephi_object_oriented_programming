#pragma once

#ifndef SHARED_HPP
#define SHARED_HPP

#include <exception>
#include <iostream>
#include <memory>

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

template <typename TO, typename FROM>
std::unique_ptr<TO> static_unique_pointer_cast(std::unique_ptr<FROM> &&old) {
    return std::unique_ptr<TO>{static_cast<TO *>(old.release())};
    // conversion: unique_ptr<FROM>->FROM*->TO*->unique_ptr<TO>
}

#endif  // SHARED_HPP
