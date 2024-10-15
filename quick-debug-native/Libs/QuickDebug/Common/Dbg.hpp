#pragma once

#include <string>
#include <span>
#include <array>
#include <charconv>


#ifdef _WIN32

// Insert function that prints the actual string
#define DEBUG_PRINT_NOARGS(x) std::cout << x

// Insert function that prints the actual string with parameters
#define DEBUG_PRINT(x, ...) printf(x, __VA_ARGS__)

#define PRINT_STRING "%s"
#define PRINT_INT64 "%lld"
#define PRINT_INT "%d"

#elif ANDROID

#include <android/log.h>

#define DEBUG_PRINT_NOARGS(x) __android_log_print(ANDROID_LOG_VERBOSE, "QD", x)
#define DEBUG_PRINT(x, ...) __android_log_print(ANDROID_LOG_VERBOSE, "QD", x, __VA_ARGS__)

#define PRINT_STRING "%s"
#define PRINT_INT64 "%ld"
#define PRINT_INT "%ld"
#endif