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
halloc user API.

Halloc allocates requested memory in a `calloc`-like manner.

Params:
    struct (type of the struct, can be also one of primitive types e.g. double)
    units (allocation count)

Returns:
    pointer to starting address of requested memory if allocation succeeded,
    NULL-pointer otherwise.

Examples:
    following cases are valid calls for the halloc:
    1) halloc(typeX, 1), where `typeX` is a type defined with typedef keyword
    2) halloc(double, 1)
    3) halloc(unsigned int, 1)
    4) halloc(struct StructName, 1)
*/

#define halloc(struct, units) (_halloc(#struct, sizeof(struct), units))


/*
hfree user API.

Hfree deallocates previously allocated memory.

Params:
    pointer to allocated data (i.e., the starting address)
*/

#define hfree(data) (_hfree(data))


/*
Memory statistics user APIs.

See currently saved page items (struct types), total memory usage by halloc or
detailed memory footprint for a specific struct type.
*/

#define halloc_print_saved_page_items() (_print_saved_page_items())

#define halloc_print_total_memory_usage() (_print_total_memory_usage())

#define halloc_print_type_memory_usage(struct) (_print_type_memory_usage(#struct))

#endif /* __HALLOC__ */
