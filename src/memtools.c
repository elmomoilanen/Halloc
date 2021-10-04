#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <memory.h>
#include <unistd.h>

#define __USE_MISC
#include <sys/mman.h>

#include "dll.h"
#include "memtools.h"


static size_t SYSTEM_PAGE_SIZE = 0;
static size_t MAX_PAGE_UNITS = 0;
static vm_page_item_container_t *first_vm_page_item_container = NULL;

void _set_system_page_size()
{
    long page_size = sysconf(_SC_PAGESIZE);

    if (page_size == -1) {
        fprintf(stderr, "%s(): %s\n", __func__, strerror(errno));
    }

    if (page_size < SYS_MIN_PAGE_SIZE) {
        fprintf(stderr, "%s(): system page size %ld doesn't meet the required minimum %d\n",
        __func__, page_size, SYS_MIN_PAGE_SIZE);
        
        exit(EXIT_FAILURE);
    }

    SYSTEM_PAGE_SIZE = page_size;
    MAX_PAGE_UNITS = MAX_SINGLE_PAGE_SIZE_BYTES / SYSTEM_PAGE_SIZE;
}

size_t _get_page_max_available_memory(size_t units)
{
    size_t const total_page_size = SYSTEM_PAGE_SIZE * units;

    return (
        total_page_size > GET_FIELD_OFFSET(vm_page_t, page_memory)
    ) ? total_page_size - GET_FIELD_OFFSET(vm_page_t, page_memory) : 0;
}

size_t _get_max_page_items_per_page_container()
{
    return MAX_PAGE_ITEMS_PER_PAGE_CONTAINER;
}

size_t _get_system_page_size()
{
    return SYSTEM_PAGE_SIZE;
}

size_t _get_max_page_units()
{
    return MAX_PAGE_UNITS;
}

static void* _create_memory_mapping(size_t units)
{
    char *vm_page = mmap(
        NULL,
        units * SYSTEM_PAGE_SIZE,
        PROT_READ|PROT_WRITE|PROT_EXEC,
        MAP_ANONYMOUS|MAP_PRIVATE,
        0, 0);

    if(vm_page == MAP_FAILED) {
        fprintf(stderr, "%s(): virtual memory mapping failed.\n", __func__);
        perror("mmap: ");
        return NULL;
    }

    memset(vm_page, 0, units * SYSTEM_PAGE_SIZE);
    return (void*) vm_page;
}

static void _delete_memory_mapping(void *addr, size_t units)
{
    if(munmap(addr, units * SYSTEM_PAGE_SIZE) == -1) {
        fprintf(stderr, "%s(): deletion of virtual memory mapping failed.\n", __func__);
        perror("munmap: ");
    }
}

vm_page_item_t* _lookup_page_item(const char *struct_name)
{
    vm_page_item_container_t *vm_page_item_container = first_vm_page_item_container;

    TRAVERSE_PAGE_CONTAINERS_BEGIN(vm_page_item_container)
    {
      vm_page_item_t *vm_page_item = vm_page_item_container->vm_page_items;

      TRAVERSE_PAGE_ITEMS_BEGIN(vm_page_item)
      {
        if(strncmp(vm_page_item->struct_name, struct_name, MAX_STRUCT_NAME_SIZE) == 0) {
            return vm_page_item;
        }
      }
      TRAVERSE_PAGE_ITEMS_END(vm_page_item);
    }
    TRAVERSE_PAGE_CONTAINERS_END(vm_page_item_container);

    return NULL;
}

static void _register_page_item_to_first_container(const char *struct_name, uint32_t struct_size)
{
    first_vm_page_item_container = _create_memory_mapping(1);
    if (first_vm_page_item_container == NULL) return;
    first_vm_page_item_container->next = NULL;

    strncpy(first_vm_page_item_container->vm_page_items->struct_name, struct_name, MAX_STRUCT_NAME_SIZE);

    first_vm_page_item_container->vm_page_items->struct_size = struct_size;
    first_vm_page_item_container->vm_page_items->first_page = NULL;
    _init_node(&first_vm_page_item_container->vm_page_items->heap_root_node);
}

void _register_page_item(const char *struct_name, uint32_t struct_size)
{
    if(first_vm_page_item_container == NULL) {
        _register_page_item_to_first_container(struct_name, struct_size);
        return;
    }

    uint32_t counter = 0;
    vm_page_item_t *vm_page_item = first_vm_page_item_container->vm_page_items;

    TRAVERSE_PAGE_ITEMS_BEGIN(vm_page_item)
    {
        if(strncmp(vm_page_item->struct_name, struct_name, MAX_STRUCT_NAME_SIZE) != 0) {
            ++counter;
        }
    }
    TRAVERSE_PAGE_ITEMS_END(vm_page_item);

    if(counter == MAX_PAGE_ITEMS_PER_PAGE_CONTAINER) {
        vm_page_item_container_t *new_vm_page_item_container = _create_memory_mapping(1);
        if (new_vm_page_item_container == NULL) return;

        new_vm_page_item_container->next = first_vm_page_item_container;
        first_vm_page_item_container = new_vm_page_item_container;

        vm_page_item = first_vm_page_item_container->vm_page_items;
    }

    strncpy(vm_page_item->struct_name, struct_name, MAX_STRUCT_NAME_SIZE);
    vm_page_item->struct_size = struct_size;
    vm_page_item->first_page = NULL;
    _init_node(&vm_page_item->heap_root_node);
}

static bool_t _is_vm_page_empty(vm_page_t *vm_page)
{
    meta_block_t first_meta_block = vm_page->meta_block;

    if(first_meta_block.is_free) {
        if(first_meta_block.next == NULL && first_meta_block.prev == NULL) return true;
    }
    return false;
}

static void _mark_vm_page_empty(vm_page_t *vm_page)
{
    vm_page->meta_block.is_free = true;
    vm_page->meta_block.next = NULL;
    vm_page->meta_block.prev = NULL;
}

static meta_block_t* _get_largest_free_meta_block(vm_page_item_t* vm_page_item)
{
    dll_node_t *largest_free_block = vm_page_item->heap_root_node.next;

    if(largest_free_block) {
        return (meta_block_t *)((char *)largest_free_block - (char *)&(((meta_block_t *)0)->heap_node));
    }
    return NULL;
}

static vm_page_t* _allocate_vm_page(vm_page_item_t *vm_page_item, uint32_t alloc_size)
{
    uint32_t const required_page_count = alloc_size/SYSTEM_PAGE_SIZE + 1;
    vm_page_t *vm_page = _create_memory_mapping(required_page_count);

    if (vm_page == NULL) return NULL;

    _mark_vm_page_empty(vm_page);

    vm_page->meta_block.block_size = _get_page_max_available_memory(required_page_count);

    if (vm_page->meta_block.block_size == 0) {
        _delete_memory_mapping((void *)vm_page, required_page_count);
        return NULL;
    }

    vm_page->system_page_count = required_page_count;

    vm_page->meta_block.offset = GET_FIELD_OFFSET(vm_page_t, meta_block);
    _init_node(&vm_page->meta_block.heap_node);

    vm_page->prev = NULL;
    vm_page->next = NULL;
    vm_page->page_item = vm_page_item;

    if(vm_page_item->first_page == NULL) {
        vm_page_item->first_page = vm_page;
    }
    else {
        vm_page_item->first_page->prev = vm_page;
        vm_page->next = vm_page_item->first_page;
        vm_page_item->first_page = vm_page;
    }
    return vm_page;
}

int16_t _compare_free_block_sizes(void *meta_block_lhs, void *meta_block_rhs)
{
    return (
        ((meta_block_t *)meta_block_lhs)->block_size > ((meta_block_t *)meta_block_rhs)->block_size
    ) ? -1 : 1;
}

static void _update_meta_block_bindings(meta_block_t *alloc_meta_block, meta_block_t *free_meta_block)
{
    free_meta_block->next = alloc_meta_block->next;
    if(free_meta_block->next != NULL) free_meta_block->next->prev = free_meta_block;

    free_meta_block->prev = alloc_meta_block;
    alloc_meta_block->next = free_meta_block;
}

static bool_t _split_free_data_block_for_allocation(
    vm_page_item_t *vm_page_item,
    meta_block_t *meta_block,
    uint32_t alloc_size)
{   
    if(alloc_size > meta_block->block_size) {
        // should not land here, if program logic ok
        return false;
    }

    uint32_t remain_size = meta_block->block_size - alloc_size;

    meta_block->is_free = false;
    meta_block->block_size = alloc_size;
    // this is safe, node here is never a head node of the priority queue
    _unlink_node(&meta_block->heap_node);

    if(remain_size < sizeof(meta_block_t)) {
        // Hard internal fragmentation, residual block without a meta block
        return true;
    }

    if(remain_size < (sizeof(meta_block_t) + vm_page_item->struct_size)) {
        // Soft internal fragmentation, meta block has a residual data block
    }

    meta_block_t *next_meta_block = NEXT_META_BLOCK_BY_SIZE(meta_block);
    next_meta_block->is_free = true;
    next_meta_block->block_size = remain_size - sizeof(meta_block_t);
    next_meta_block->offset = meta_block->offset + sizeof(meta_block_t) + meta_block->block_size;

    _init_node(&next_meta_block->heap_node);

    _add_to_priority_queue(
        &vm_page_item->heap_root_node,
        &next_meta_block->heap_node,
        GET_FIELD_OFFSET(meta_block_t, heap_node),
        &_compare_free_block_sizes
    );

    _update_meta_block_bindings(meta_block, next_meta_block);

    return true;
}


meta_block_t* _allocate_free_data_block(vm_page_item_t *vm_page_item, uint32_t alloc_size)
{
    meta_block_t *largest_free_meta_block = _get_largest_free_meta_block(vm_page_item);

    if(largest_free_meta_block == NULL || largest_free_meta_block->block_size < alloc_size) {
        vm_page_t *vm_page = _allocate_vm_page(vm_page_item, alloc_size);

        if (vm_page == NULL) return NULL;

        _add_to_priority_queue(
            &vm_page_item->heap_root_node,
            &vm_page->meta_block.heap_node,
            GET_FIELD_OFFSET(meta_block_t, heap_node),
            &_compare_free_block_sizes
        );
        
        if(_split_free_data_block_for_allocation(vm_page_item, &vm_page->meta_block, alloc_size)) {
            return &vm_page->meta_block;
        }
        return NULL;
    }

    if(_split_free_data_block_for_allocation(vm_page_item, largest_free_meta_block, alloc_size)) {
        return largest_free_meta_block;
    }
    return NULL;
}


static void _merge_free_data_blocks(meta_block_t *meta_block_lhs, meta_block_t *meta_block_rhs)
{
    meta_block_lhs->next = meta_block_rhs->next;
    meta_block_lhs->block_size += sizeof(meta_block_t) + meta_block_rhs->block_size;

    if(meta_block_rhs->next != NULL) meta_block_rhs->next->prev = meta_block_lhs;
}

static void _free_vm_page(vm_page_t *vm_page)
{
    vm_page_item_t *vm_page_item = vm_page->page_item;

    if(vm_page_item->first_page == vm_page) {
        vm_page_item->first_page = vm_page->next;
        if(vm_page->next) vm_page->next->prev = NULL;
        vm_page->next = NULL;
        vm_page->prev = NULL;
    }
    else {
        if(vm_page->next) vm_page->next->prev = vm_page->prev;
        vm_page->prev->next = vm_page->next;
    }

    if(vm_page_item->first_page == NULL) {
        vm_page_item->heap_root_node.next = NULL;
    }

    _delete_memory_mapping((void *)vm_page, vm_page->system_page_count);
}


void _free_data_blocks(meta_block_t *meta_block)
{
    meta_block_t *updated_lowest_meta_block = meta_block;
    vm_page_t *vm_page = GET_META_PAGE(meta_block, meta_block->offset);
    meta_block->is_free = true;

    meta_block_t *next_meta_block = NEXT_META_BLOCK(meta_block);

    if(next_meta_block == NULL) {
        char *vm_page_end_addr = (char *)vm_page + vm_page->system_page_count * SYSTEM_PAGE_SIZE;
        char *data_block_end_addr = (char *)(meta_block + 1) + meta_block->block_size;
        
        meta_block->block_size += (uint32_t) ((uint64_t)vm_page_end_addr - (uint64_t)data_block_end_addr);
    }
    else {
        meta_block_t *next_meta_block_by_size = NEXT_META_BLOCK_BY_SIZE(meta_block);
        meta_block->block_size += (uint32_t) ((uint64_t)next_meta_block - (uint64_t)next_meta_block_by_size);
    }

    if(next_meta_block && next_meta_block->is_free == true) {
        _merge_free_data_blocks(meta_block, next_meta_block);
        updated_lowest_meta_block = meta_block;
    }

    meta_block_t *prev_meta_block = PREV_META_BLOCK(meta_block);

    if(prev_meta_block && prev_meta_block->is_free) {
        _merge_free_data_blocks(prev_meta_block, meta_block);
        updated_lowest_meta_block = prev_meta_block;
    }

    if(_is_vm_page_empty(vm_page)) {
        _unlink_node(&vm_page->meta_block.heap_node);
        _free_vm_page(vm_page);
    }
    else {
        _add_to_priority_queue(
            &vm_page->page_item->heap_root_node,
            &updated_lowest_meta_block->heap_node,
            GET_FIELD_OFFSET(meta_block_t, heap_node),
            &_compare_free_block_sizes
        );
    }
}


void _walk_vm_page_items()
{
    vm_page_item_container_t *vm_page_item_container = first_vm_page_item_container;
    uint32_t page_counter = 0;

    TRAVERSE_PAGE_CONTAINERS_BEGIN(vm_page_item_container)
    {
        printf("vm page container %u : %p\n\n",
        page_counter + 1, (void *)vm_page_item_container);

        vm_page_item_t *vm_page_item = vm_page_item_container->vm_page_items;
        uint32_t page_item_counter = 0; 

        TRAVERSE_PAGE_ITEMS_BEGIN(vm_page_item)
        {
            printf("vm page item %u : %p\n",
            page_item_counter + 1, (void *)vm_page_item);

            printf("item name `%s`, size `%u` bytes \n\n",
            vm_page_item->struct_name, vm_page_item->struct_size);

            ++page_item_counter;
        }
        TRAVERSE_PAGE_ITEMS_END(vm_page_item);

        ++page_counter;
    }
    TRAVERSE_PAGE_CONTAINERS_END(vm_page_item_container);
}


void _print_memory_usage()
{
    vm_page_item_container_t *vm_page_item_container = first_vm_page_item_container;

    TRAVERSE_PAGE_CONTAINERS_BEGIN(vm_page_item_container)
    {
        vm_page_item_t *vm_page_item = vm_page_item_container->vm_page_items;

        TRAVERSE_PAGE_ITEMS_BEGIN(vm_page_item)
        {
            uint32_t total_block_count = 0, free_block_count = 0;
            uint32_t memory_usage = 0;

            vm_page_t *vm_page = vm_page_item->first_page;

            TRAVERSE_PAGES_BEGIN(vm_page)
            {
                meta_block_t *meta_block = &vm_page->meta_block;

                TRAVERSE_META_BLOCKS_IN_PAGE_BEGIN(meta_block)
                {
                    total_block_count += 1;
                    if(meta_block->is_free == true) free_block_count += 1;
                    else memory_usage += meta_block->block_size + sizeof(meta_block_t);
                }
                TRAVERSE_META_BLOCKS_IN_PAGE_END(meta_block);
            }
            TRAVERSE_PAGES_END(vm_page);

            printf("struct: %-32s    blocks: %-5u    free blocks: %-5u   used memory in bytes: %u\n\n",
            vm_page_item->struct_name, total_block_count, free_block_count, memory_usage);
        }
        TRAVERSE_PAGE_ITEMS_END(vm_page_item);
    }
    TRAVERSE_PAGE_CONTAINERS_END(vm_page_item_container);
}


void _walk_vm_pages(const char *struct_name)
{
    vm_page_item_t* vm_page_item = _lookup_page_item(struct_name);

    if(vm_page_item == NULL) {
        fprintf(stderr, "%s(): struct `%s` hasn't been registered yet.\n", __func__, struct_name);
        return;
    }

    vm_page_t *vm_page = vm_page_item->first_page;

    TRAVERSE_PAGES_BEGIN(vm_page)
    {
        meta_block_t *meta_block = &vm_page->meta_block;

        uint32_t free_data_blocks = 0, allocated_data_blocks = 0;
        meta_block_t *meta_block_with_largest_free_data_block = NULL;
        meta_block_t *meta_block_with_largest_allocated_data_block = NULL;

        TRAVERSE_META_BLOCKS_IN_PAGE_BEGIN(meta_block)
        {
            if(meta_block->is_free) {
                free_data_blocks++;
                if(meta_block_with_largest_free_data_block != NULL) {
                    if(meta_block->block_size > meta_block_with_largest_free_data_block->block_size) {
                        meta_block_with_largest_free_data_block = meta_block;
                    }
                }
                else meta_block_with_largest_free_data_block = meta_block;
            }
            else {
                allocated_data_blocks++;
                if(meta_block_with_largest_allocated_data_block != NULL) {
                    if(meta_block->block_size > meta_block_with_largest_allocated_data_block->block_size) {
                        meta_block_with_largest_allocated_data_block = meta_block;
                    }
                }
                else meta_block_with_largest_allocated_data_block = meta_block;
            }
        }
        TRAVERSE_META_BLOCKS_IN_PAGE_END(meta_block);

        printf("vm page %p has %u free and %u allocated data blocks\n",
        (void *)vm_page, free_data_blocks, allocated_data_blocks);

        printf("largest free data block has address %p and size %u\n",
        (void *)(meta_block_with_largest_free_data_block + 1),
        meta_block_with_largest_free_data_block->block_size);

        printf("largest allocated data block has address %p and size %u\n\n",
        (void *)(meta_block_with_largest_allocated_data_block + 1),
        meta_block_with_largest_allocated_data_block->block_size);

    }
    TRAVERSE_PAGES_END(vm_page);
}
