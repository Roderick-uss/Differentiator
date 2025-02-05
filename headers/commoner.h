#ifndef COMMON
#define COMMON

#include <stdint.h>
#include <stdio.h>
#include "colors.h"

#define LOG_CERR(  str, ...) fprintf(stderr, YELLOW str DEFAULT __VA_OPT__(,) __VA_ARGS__)
#define LOG_INFO(  str, ...) fprintf(stderr, GREEN  str DEFAULT __VA_OPT__(,) __VA_ARGS__)
#define LOG_FATAL( str, ...) fprintf(stderr, RED    str DEFAULT __VA_OPT__(,) __VA_ARGS__)

#define LOG_PURPLE(str, ...) fprintf(stderr, PURPLE str DEFAULT __VA_OPT__(,) __VA_ARGS__)
#define LOG_YELLOW(str, ...) fprintf(stderr, YELLOW str DEFAULT __VA_OPT__(,) __VA_ARGS__)
#define LOG_GREEN( str, ...) fprintf(stderr, GREEN  str DEFAULT __VA_OPT__(,) __VA_ARGS__)
#define LOG_WHITE( str, ...) fprintf(stderr, WHITE  str DEFAULT __VA_OPT__(,) __VA_ARGS__)
#define LOG_BLACk( str, ...) fprintf(stderr, BLACK  str DEFAULT __VA_OPT__(,) __VA_ARGS__)
#define LOG_BLUE(  str, ...) fprintf(stderr, BLUE   str DEFAULT __VA_OPT__(,) __VA_ARGS__)
#define LOG_CYAN(  str, ...) fprintf(stderr, CYAN   str DEFAULT __VA_OPT__(,) __VA_ARGS__)
#define LOG_RED(   str, ...) fprintf(stderr, RED    str DEFAULT __VA_OPT__(,) __VA_ARGS__)

struct string_t {
    const char* begin;
    size_t size;
};

void swap_memory   (void* x1, void* x2, size_t size);
void swap_memory_2 (void* x1, void* x2, size_t size);

uint64_t ceil_mod_8 (uint64_t num);

int   run_file_error  (const char* file_name);
void* get_file_buffer (const char* file_name);

#endif // COMMON
