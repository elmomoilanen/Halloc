#ifndef __COMMON__
#define __COMMON__

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_RESET "\x1b[0m"

#define PRINT_SUCCESS(func) printf("%s() %spassed%s\n", (func),\
    ANSI_COLOR_GREEN, ANSI_COLOR_RESET)

typedef bool bool_t;
typedef int i32;
typedef unsigned int u32;
typedef long i64;
typedef unsigned long u64;

void init_memtools_testing();

typedef struct {
    char *name;
    void (*func)();
} test_func;

extern test_func dll_tests[];
extern test_func memtools_tests[];
extern test_func halloc_tests[];

#endif /* __COMMON__ */
