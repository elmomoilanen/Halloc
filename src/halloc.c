#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "memtools.h"
#include "halloc.h"


/*
Do not increase the `MAX_PAGE_UNITS` unless taking care of necessary modifications.
Assumption is that for one allocation, required memory bytes are less that 2^32 - 1.
*/
static size_t MAX_PAGE_UNITS = 262145;


void* _halloc(char *struct_name, uint32_t struct_size, size_t units)
{
    if(units < 1)
    {
        fprintf(stderr, "%s(): minimum allocation units equals 1.\n", __func__);
        return NULL;
    }

    if(strlen(struct_name) >= MAX_STRUCT_NAME_SIZE)
    {
        uint32_t const max_size = MAX_STRUCT_NAME_SIZE;
        fprintf(stderr, "%s(): struct name is not allowed to be larger than %u characters.\n", __func__, max_size-1);
        return NULL;
    }

    _set_system_page_size();
    uint32_t const max_mem = _get_page_max_available_memory(MAX_PAGE_UNITS);

    if(struct_size * units > max_mem)
    {
        fprintf(stderr, "%s(): requested memory allocation size exceeds implementation limit of %u bytes.\n", __func__, max_mem);
        return NULL;
    }

    vm_page_item_t *vm_page_item = _lookup_page_item(struct_name);

    if(vm_page_item == NULL)
    {
        _register_page_item(struct_name, struct_size);
        vm_page_item = _lookup_page_item(struct_name);

        if(vm_page_item == NULL)
        {
            fprintf(stderr, "%s(): structure %s registeration failed.\n", __func__, struct_name);
            return NULL;
        }
    }

    meta_block_t *free_meta_block = _allocate_free_data_block(vm_page_item, units * vm_page_item->struct_size);

    if(free_meta_block != NULL)
    {
        memset((char *)(free_meta_block + 1), 0, free_meta_block->block_size);
        return (void *)(free_meta_block + 1);
    }

    return NULL;
}


void _hfree(void* data)
{
    if(data == NULL) return;

    meta_block_t *meta_block = (meta_block_t *)((char *)data - sizeof(meta_block_t));
    
    _free_data_blocks(meta_block);
}


void _print_saved_page_items()
{
    printf("currently saved virtual memory page items...\n");
    _walk_vm_page_items();
}


void _print_total_memory_usage()
{
    printf("showing total memory usage by halloc...\n");
    _print_memory_usage();
}


void _print_type_memory_usage(char *struct_name)
{
    printf("showing detailed memory usage for type %s...\n\n", struct_name);
    _walk_vm_pages(struct_name);
}
