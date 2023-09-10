#ifdef __has_feature
#if __has_feature(address_sanitizer)
#define ASAN_ENABLED 1
#endif
#endif

#if ASAN_ENABLED && (defined(_WIN64) || defined(_WIN32))
/* SEE: https://github.com/google/sanitizers/issues/749 */
#define TRY_CATCH_WRAPPER(code) [&]() __attribute__((no_sanitize_address)){code}();
#else
#define TRY_CATCH_WRAPPER(code) code
#endif
