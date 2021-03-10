#include "common.h"

#include "dll.h"
#include "memtools.h"
#include "halloc.h"

#define halloc(struct, units) (_halloc(#struct, sizeof(struct), units))
#define hfree(data) (_hfree(data))


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


static void test_allocation_huge()
{
    u32 const alloc_count = 500000;

    product *p = halloc(product, alloc_count);

    assert(p != NULL);

    assert(p[100].year == 0);
    assert(p[alloc_count - 1].year == 0);

    hfree(p);

    PRINT_SUCCESS(__func__);

}


static void test_allocation_oversize()
{
    u32 const alloc_count = 50000000;

    product *p = halloc(product, alloc_count);

    assert(p == NULL);

    PRINT_SUCCESS(__func__);
}


test_func halloc_tests[] = {
    {"allocation_small", test_allocation_small},
    {"allocation_medium", test_allocation_medium},
    {"allocation_large", test_allocation_large},
    {"allocation_huge", test_allocation_huge},
    {"allocation_oversize", test_allocation_oversize},
    {NULL, NULL},
};
