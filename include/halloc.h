#ifndef __HALLOC__
#define __HALLOC__

#include <stdint.h>
#include <stddef.h>

void* _halloc(char *struct_name, uint32_t struct_size, size_t units);
void _hfree(void* data);

void _print_saved_page_items();
void _print_total_memory_usage();
void _print_type_memory_usage(char *struct_name);

/*
Halloc memory allocator.

Params:
    struct: type of the struct, can also be directly one of the primitive types, e.g. int or double
    units: allocation count

Returns:
    void pointer or NULL: pointer to the starting address of the allocated memory if allocation succeeded,
        NULL-pointer otherwise.

Examples:
    1) halloc(myType, 1), where `myType` is a custom type defined with the typedef keyword
    2) halloc(double, 5)
    3) halloc(unsigned int, 1)
    4) halloc(struct StructName, 2)
*/

#define halloc(struct, units) (_halloc(#struct, sizeof(struct), units))

/*
Hfree deallocates previously allocated memory by halloc.

Params:
    data: pointer to the starting address of the allocated data.

Examples:
    // allocate 5 units of myType which is now a custom type defined with typedef keyword
    ptr = halloc(myType, 5)
    // if ptr != NULL, do something with it...
    // at the end, deallocate memory with hfree
    hfree(ptr)
*/

#define hfree(data) (_hfree(data))

/*
Virtual memory statistics APIs.

Prints currently saved page items (struct types), total memory usage by halloc or
detailed memory footprint for a specific struct type.
*/

#define halloc_print_saved_page_items() (_print_saved_page_items())

#define halloc_print_total_memory_usage() (_print_total_memory_usage())

#define halloc_print_type_memory_usage(struct) (_print_type_memory_usage(#struct))


#endif /* __HALLOC__ */
