#ifndef __HALLOC__
#define __HALLOC__

#include <stdint.h>
#include <stddef.h>


/*
halloc user API.

Halloc allocates requested memory in a `calloc`-like manner.

Params:
    struct (type of the struct)
    units (allocation count)

Returns:
    pointer to starting address of requested memory if allocation succeeded,
    NULL-pointer otherwise.
*/

void* _halloc(char *struct_name, uint32_t struct_size, size_t units);
#define halloc(struct, units) (_halloc(#struct, sizeof(struct), units)) 


/*
hfree user API.

Hfree deallocated previously allocated memory.

Params:
    pointer to allocated data (i.e., the starting address)
*/

void _hfree(void* data);
#define hfree(data) (_hfree(data));


/*
Memory statistics user APIs.

See currently saved page items (struct types), total memory usage by halloc or
detailed memory footprint for a specific struct type.
*/

void print_saved_page_items();
void print_total_memory_usage();

void _print_type_memory_usage(char *struct_name);
#define print_type_memory_usage(struct) (_print_type_memory_usage(#struct))


#endif /* __HALLOC__ */
