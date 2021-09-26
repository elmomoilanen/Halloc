#ifndef __MEMTOOLS__
#define __MEMTOOLS__

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "dll.h"


#define MAX_STRUCT_NAME_SIZE 64
#define SYS_MIN_PAGE_SIZE 4096

typedef bool bool_t;

typedef struct meta_block_ {
  bool_t is_free;
  uint32_t block_size;
  uint32_t offset;
  dll_node_t heap_node;
  struct meta_block_ *prev;
  struct meta_block_ *next;
} meta_block_t;

struct vm_page_item_;

typedef struct vm_page_ {
    struct vm_page_ *prev;
    struct vm_page_ *next;
    struct vm_page_item_ *page_item;
    uint32_t system_page_count;
    meta_block_t meta_block;
    char page_memory[];
} vm_page_t;

typedef struct vm_page_item_ {
    char struct_name[MAX_STRUCT_NAME_SIZE];
    uint32_t struct_size;
    vm_page_t *first_page;
    dll_node_t heap_root_node;
 } vm_page_item_t;

typedef struct vm_page_item_container_ {
    struct vm_page_item_container_ *next;
    vm_page_item_t vm_page_items[];
 } vm_page_item_container_t;


#define GET_FIELD_OFFSET(struct, field) ((size_t)(&((struct *)0)->field))

#define GET_META_PAGE(meta_block, offset) ((void *)((char *)meta_block - offset))

#define NEXT_META_BLOCK(meta_block) (meta_block->next)

#define PREV_META_BLOCK(meta_block) (meta_block->prev)

#define NEXT_META_BLOCK_BY_SIZE(meta_block) ((meta_block_t *)((char *)(meta_block + 1) + meta_block->block_size))

#define MAX_PAGE_ITEMS_PER_PAGE_CONTAINER ((SYSTEM_PAGE_SIZE - sizeof(vm_page_item_container_t *)) / sizeof(vm_page_item_t))


#define TRAVERSE_PAGE_CONTAINERS_BEGIN(vm_page_item_container)  \
{   \
    vm_page_item_container_t *_vm_page_item_container = NULL;   \
    for(; vm_page_item_container != NULL; vm_page_item_container = _vm_page_item_container){    \
        _vm_page_item_container = vm_page_item_container->next;

#define TRAVERSE_PAGE_CONTAINERS_END(vm_page_item_container) }}


#define TRAVERSE_PAGE_ITEMS_BEGIN(vm_page_item) \
{   \
    size_t count = 0;   \
    for(; count < MAX_PAGE_ITEMS_PER_PAGE_CONTAINER && vm_page_item->struct_size; vm_page_item++, count++){ \

#define TRAVERSE_PAGE_ITEMS_END(vm_page_item) }}


#define TRAVERSE_PAGES_BEGIN(vm_page)   \
{   \
    vm_page_t *_vm_page = NULL; \
    for(; vm_page != NULL; vm_page = _vm_page){ \
        _vm_page = vm_page->next;

#define TRAVERSE_PAGES_END(vm_page) }}


#define TRAVERSE_META_BLOCKS_IN_PAGE_BEGIN(meta_block)  \
{   \
    meta_block_t *_meta_block = NULL;   \
    for(; meta_block != NULL; meta_block = _meta_block){    \
        _meta_block = meta_block->next;

#define TRAVERSE_META_BLOCKS_IN_PAGE_END(meta_block) }}


void _set_system_page_size();
size_t _get_page_max_available_memory(size_t units);
size_t _get_max_page_items_per_page_container();
size_t _get_system_page_size();

vm_page_item_t* _lookup_page_item(const char *struct_name);
void _register_page_item(const char *struct_name, uint32_t struct_size);

meta_block_t* _allocate_free_data_block(vm_page_item_t *vm_page_item, uint32_t alloc_size);
void _free_data_blocks(meta_block_t *meta_block);

void _walk_vm_page_items();
void _print_memory_usage();
void _walk_vm_pages(const char *struct_name);


#endif /* __MEMTOOLS__ */
