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

typedef struct {
    u32 *data;
    u32 size;
} typeA;

typedef struct {
  double *data;
  unsigned int size;
} typeX;


static void test_allocation_primitive_type_small()
{
    u32 const alloc_count = 25;

    i32 *ptr = halloc(i32, alloc_count);
    assert(ptr != NULL);

    assert(ptr[0] == 0);
    assert(ptr[alloc_count - 1] == 0);

    hfree(ptr);

    PRINT_SUCCESS(__func__);
}


static void test_allocation_primitive_type_large()
{
    u32 const alloc_count = 10000;

    i32 *ptr = halloc(i32, alloc_count);
    assert(ptr != NULL);

    assert(ptr[0] == 0);
    assert(ptr[alloc_count - 1] == 0);

    hfree(ptr);

    PRINT_SUCCESS(__func__);
}


static void test_allocation_primitive_type_with_space()
{
    u32 const alloc_count = 1;

    unsigned int *ptr = halloc(unsigned int, alloc_count);
    assert(ptr != NULL);

    hfree(ptr);

    PRINT_SUCCESS(__func__);
}


static void test_allocation_for_too_large_struct_name()
{
    typedef struct {
        char name[25];
        i32 x;
    } ToooLongStructTypeNameToBeValidAndForCurrentHallocImplementation;

    ToooLongStructTypeNameToBeValidAndForCurrentHallocImplementation *type_ptr =\
        halloc(ToooLongStructTypeNameToBeValidAndForCurrentHallocImplementation, 1);

    assert(type_ptr == NULL);

    PRINT_SUCCESS(__func__);
}


static void test_allocation_large_struct_name()
{
    typedef struct {
        char name[25];
        i32 x;
    } LoongStructTypeNameButOkToBeValidForCurrentHallocImplementation;

    LoongStructTypeNameButOkToBeValidForCurrentHallocImplementation *type_ptr =\
        halloc(LoongStructTypeNameButOkToBeValidForCurrentHallocImplementation, 1);

    assert(type_ptr != NULL);
    assert(type_ptr->x == 0);

    hfree(type_ptr);

    PRINT_SUCCESS(__func__);
}


static void test_allocation_small()
{
    product *p = halloc(product, 1);

    assert(p != NULL);
    assert(p->volume == 0);

    hfree(p);

    PRINT_SUCCESS(__func__);
}


static void test_allocation_small_other()
{
    u32 const alloc_count = 25;

    product *p = halloc(product, alloc_count);

    assert(p != NULL);

    for(u32 j=0; j<alloc_count; ++j)
    {
        assert( (p+j) != NULL );
        assert( (*(p+j)).year == 0 );
        assert( (*(p+j)).volume == 0 );
    }

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
    assert(p[100].weight == 0);
    assert(p[100].volume == 0);

    assert(p[100000].weight == 0);
    assert(p[100000].year == 0);
    assert(p[100000].volume == 0);

    assert(p[alloc_count - 1].year == 0);
    assert(p[alloc_count - 1].weight == 0);
    assert(p[alloc_count - 1].volume == 0);

    hfree(p);

    PRINT_SUCCESS(__func__);

}


static void test_allocation_oversize()
{
    u32 const alloc_count = 50000000;

    product *p = halloc(product, alloc_count);

    // allocation should fail due to its size, an error message should also appear in stdout
    assert(p == NULL);

    PRINT_SUCCESS(__func__);
}


static void test_nested_allocation()
{
    typeA *typeA = halloc(typeA, 1);
    assert(typeA != NULL);
    assert(typeA->data == 0);

    typeA->size = 50;

    typeA->data = halloc(u32, typeA->size);
    assert(typeA->data != NULL);

    assert(typeA->data[0] == 0);
    assert(typeA->data[typeA->size - 1] == 0);

    hfree(typeA->data);
    hfree(typeA);

    PRINT_SUCCESS(__func__);
}


static void test_readme_example_allocation()
{
    typeX *p = halloc(typeX, 1);
    assert(p != NULL);
    
    p->size = 25;
    p->data = halloc(double, p->size);
    assert(p->data != NULL);

    p->data[p->size - 1] = 0.0;
  
    hfree(p->data);
    hfree(p);

    PRINT_SUCCESS(__func__);
}


test_func halloc_tests[] = {
    {"allocation_primitive_type_small", test_allocation_primitive_type_small},
    {"allocation_primitive_type_large", test_allocation_primitive_type_large},
    {"allocation_primitive_type_with_space", test_allocation_primitive_type_with_space},
    {"allocation_for_too_large_struct_name", test_allocation_for_too_large_struct_name},
    {"allocation_large_struct_name", test_allocation_large_struct_name},
    {"allocation_small", test_allocation_small},
    {"allocation_small_other", test_allocation_small_other},
    {"allocation_medium", test_allocation_medium},
    {"allocation_large", test_allocation_large},
    {"allocation_huge", test_allocation_huge},
    {"allocation_oversize", test_allocation_oversize},
    {"nested_allocation", test_nested_allocation},
    {"readme_example_allocation", test_readme_example_allocation},
    {NULL, NULL},
};
