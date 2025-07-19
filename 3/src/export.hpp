#pragma once

#ifndef EXPORT_HPP
#define EXPORT_HPP

#if defined(_WIN32) || defined(__CYGWIN__)
    #ifdef GAME_LOGIC_EXPORTS
        #ifdef __GNUC__
            #define GAME_API __attribute__((dllexport))
        #else
            #define GAME_API __declspec(dllexport)
        #endif
    #else
        #ifdef __GNUC__
            #define GAME_API __attribute__((dllimport))
        #else
            #define GAME_API __declspec(dllimport)
        #endif
    #endif
    #define GAME_LOCAL
#else
    #if __GNUC__ >= 4
        #define GAME_API __attribute__ ((visibility ("default")))
        #define GAME_LOCAL  __attribute__ ((visibility ("hidden")))
    #else
        #define GAME_API
        #define GAME_LOCAL
    #endif
#endif

#endif // EXPORT_HPP
