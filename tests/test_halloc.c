#include "common.h"

#include "dll.h"
#include "memtools.h"
#include "halloc.h"


typedef struct {
    char name[256];
    u32 year;
    u32 weight;
    u32 volume;
} product;


static void test_allocation_small()
{
    product *p = halloc(product, 1);

    assert(p != NULL);
    assert(p->volume == 0);

    hfree(p);

    PRINT_SUCCESS(__func__);
}


static void test_allocation_medium()
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


static void test_allocation_large()
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


test_func halloc_tests[] = {
    {"allocation_small", test_allocation_small},
    {"allocation_medium", test_allocation_medium},
    {"allocation_large", test_allocation_large},
    {NULL, NULL},
};
