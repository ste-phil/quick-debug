#pragma once
#pragma once

#include <string>
#include <span>
#include <array>
#include <charconv>

// Insert function that prints the actual string
#define DEBUG_PRINT_NOARGS(x) printf(x)

// Insert function that prints the actual string with parameters
#define DEBUG_PRINT(x, ...) printf(x, __VA_ARGS__)

// Placeholder for strings (e.g. %s for printf, {} for spdlog)
#define PRINT_STRING "%s"

// Placeholder for ints (e.g. %d for printf, {} for spdlog)
#define PRINT_INT64 "%d"
