#ifndef __HALLOC__
#define __HALLOC__

#include <stdint.h>
#include <stddef.h>


void* _halloc(char *struct_name, uint32_t struct_size, size_t units);
void _hfree(void* data);

void _print_saved_page_items();
void _print_total_memory_usage();

void _print_type_memory_usage(char *struct_name);

#endif /* __HALLOC__ */
