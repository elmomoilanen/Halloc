#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "dll.h"
#include "memtools.h"
#include "halloc.h"

#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_RESET "\x1b[0m"

#define PRINT_SUCCESS(func) printf("%s() %spassed%s\n", (func),\
    ANSI_COLOR_GREEN, ANSI_COLOR_RESET)


typedef int i32;
typedef unsigned int u32;
typedef long i64;
typedef unsigned long u64; 


typedef struct {
    char name[256];
    u32 year;
    u32 weight;
    u32 volume;
} product;


void test_allocation_small()
{
    product *p = halloc(product, 1);

    assert(p != NULL);
    assert(p->volume == 0);

    hfree(p);

    PRINT_SUCCESS(__func__);
}


void test_allocation_medium()
{
    u32 const alloc_count = 100;

    product *p = halloc(product, alloc_count);

    assert(p != NULL);

    for(u32 j=0; j<alloc_count; ++j)
    {
        assert( (p+j) != NULL );
        assert( (*(p+j)).volume == 0 );
    }

    hfree(p);

    PRINT_SUCCESS(__func__);
}


void test_allocation_large()
{
    u32 const alloc_count = 10000;

    product *p = halloc(product, alloc_count);

    assert(p != NULL);

    for(u32 j=0; j<alloc_count; ++j)
    {
        assert( (p+j) != NULL );
        assert( (*(p+j)).volume == 0 );
    }

    hfree(p);

    PRINT_SUCCESS(__func__);
}


int main()
{
    printf("running halloc tests...\n");

    test_allocation_small();
    test_allocation_medium();
    test_allocation_large();
}
