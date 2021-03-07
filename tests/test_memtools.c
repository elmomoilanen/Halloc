#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "dll.h"
#include "memtools.h"

#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_RESET "\x1b[0m"

#define PRINT_SUCCESS(func) printf("%s() %spassed%s\n", (func),\
    ANSI_COLOR_GREEN, ANSI_COLOR_RESET)


typedef int i32;
typedef unsigned int u32;
typedef long i64;
typedef unsigned long u64; 

typedef struct {
    char header[32];
    char code[64];
    i32 data[50];
} test_x;

typedef struct {
    char header[32];
    u32 code;
    u64 udata[24];
    i64 idata[250];
    char flag[100];
} test_y;

typedef struct test_z_ {
    struct test_z_ *prev;
    struct test_z_ *next;
    u32 values[1024];    
} test_z;

// same limit as in halloc.c
static size_t MAX_PAGE_UNITS = 262145;


void test_page_item_registration()
{
    const char *struct_name = "test_x1";

    vm_page_item_t *page_item = lookup_page_item(struct_name);
    assert(page_item == NULL);

    register_page_item(struct_name, sizeof(test_x));

    page_item = lookup_page_item(struct_name);

    assert(page_item != NULL);
    assert(strncmp(page_item->struct_name, struct_name, MAX_STRUCT_NAME_SIZE) == 0);
    assert(page_item->struct_size == sizeof(test_x));

    PRINT_SUCCESS(__func__);
}


void test_page_item_registration_for_few()
{
    vm_page_item_t *page_item = lookup_page_item("test_x2");
    assert(page_item == NULL);

    register_page_item("test_x2", sizeof(test_x));
    register_page_item("test_y2", sizeof(test_y));
    register_page_item("test_z2", sizeof(test_z));

    page_item = lookup_page_item("test_y2");

    assert(page_item != NULL);
    assert(strncmp(page_item->struct_name, "test_y2", MAX_STRUCT_NAME_SIZE) == 0);
    assert(page_item->struct_size == sizeof(test_y));

    PRINT_SUCCESS(__func__);
}


void test_page_item_registration_for_multiple()
{
    uint32_t const max_containers_per_page = get_max_page_items_per_page_container();

    assert(max_containers_per_page > 0);

    i32 const max_reg_number = max_containers_per_page + 25;
    for(i32 j=1; j<=max_reg_number; ++j)
    {
        char name[12];
        snprintf(name, sizeof name, "%s_%d", "test", j);

        register_page_item(name, sizeof(test_x));
    }

    vm_page_item_t *page_item = lookup_page_item("test_1");

    assert(page_item != NULL);
    assert(page_item->struct_size == sizeof(test_x));

    char last_name[12];
    snprintf(last_name, sizeof last_name, "%s_%d", "test", max_reg_number);

    page_item = lookup_page_item(last_name);

    assert(page_item != NULL);
    assert(page_item->struct_size == sizeof(test_x));

    PRINT_SUCCESS(__func__);
}


void test_free_data_block_allocation_small_size()
{
    const char *struct_name = "test_x";
    vm_page_item_t *page_item = lookup_page_item(struct_name);
    assert(page_item != NULL);

    u32 const system_page_size = get_system_page_size();
    u32 const needed_page_count = (1*page_item->struct_size)/system_page_size + 1;
    u32 const needed_block_size = 1*page_item->struct_size;

    meta_block_t *free_meta_block = allocate_free_data_block(page_item, 1 * page_item->struct_size);

    assert(free_meta_block != NULL);
    assert(&page_item->first_page->meta_block == free_meta_block);

    assert(page_item->first_page->system_page_count == needed_page_count);
    assert(free_meta_block->block_size == needed_block_size);

    // following should already work at this point
    free_data_blocks(free_meta_block);
    assert(page_item->first_page == NULL);

    PRINT_SUCCESS(__func__);
}


void test_free_data_block_allocation_medium_size()
{
    const char *struct_name = "test_y";
    vm_page_item_t *page_item = lookup_page_item(struct_name);
    assert(page_item != NULL);

    u32 const system_page_size = get_system_page_size();
    u32 const request_count = 1000;

    if(page_item->struct_size * request_count > get_page_max_available_memory(MAX_PAGE_UNITS)) assert(0);

    u32 const needed_page_count = (request_count * page_item->struct_size) / system_page_size + 1;
    u32 const needed_block_size = request_count * page_item->struct_size;

    meta_block_t *free_meta_block = allocate_free_data_block(page_item, request_count * page_item->struct_size);

    assert(free_meta_block != NULL);
    assert(&page_item->first_page->meta_block == free_meta_block);

    assert(page_item->first_page->system_page_count == needed_page_count);
    assert(free_meta_block->block_size == needed_block_size);

    free_data_blocks(free_meta_block);
    assert(page_item->first_page == NULL);

    PRINT_SUCCESS(__func__);
}


void test_free_data_block_allocation_large_size()
{
    const char *struct_name = "test_z";
    vm_page_item_t *page_item = lookup_page_item(struct_name);
    assert(page_item != NULL);

    u32 const system_page_size = get_system_page_size();
    u32 const request_count = 100000;

    if(page_item->struct_size * request_count > get_page_max_available_memory(MAX_PAGE_UNITS)) assert(0);

    u32 const needed_page_count = (request_count * page_item->struct_size) / system_page_size + 1;
    u32 const needed_block_size = request_count * page_item->struct_size;

    meta_block_t *free_meta_block = allocate_free_data_block(page_item, request_count * page_item->struct_size);

    assert(free_meta_block != NULL);
    assert(&page_item->first_page->meta_block == free_meta_block);

    assert(page_item->first_page->system_page_count == needed_page_count);
    assert(free_meta_block->block_size == needed_block_size);

    free_data_blocks(free_meta_block);
    assert(page_item->first_page == NULL);

    PRINT_SUCCESS(__func__);
}


void test_free_data_block_allocation_for_consecutive_times()
{
    const char *struct_name = "test_x";
    vm_page_item_t *page_item = lookup_page_item(struct_name);
    assert(page_item != NULL);
    assert(page_item->first_page == NULL);

    u32 const system_page_size = get_system_page_size();
    u32 needed_page_count = (1*page_item->struct_size)/system_page_size + 1;

    meta_block_t *first_free_meta_block = allocate_free_data_block(page_item, 1 * page_item->struct_size);
    assert(first_free_meta_block != NULL);
    
    assert(page_item->first_page->system_page_count == needed_page_count);

    meta_block_t *second_free_meta_block = allocate_free_data_block(page_item, 2 * page_item->struct_size);
    assert(second_free_meta_block != NULL);

    // assuming that everything so far fits into one standard size page
    assert(page_item->first_page->next == NULL);

    assert(first_free_meta_block->next == second_free_meta_block);
    assert(second_free_meta_block->block_size == 2 * page_item->struct_size);
    assert(first_free_meta_block->next->is_free == false);

    free_data_blocks(second_free_meta_block);
    assert(first_free_meta_block->next->is_free == true);

    // assuming that following allocation needs a new page (it becomes first in order)

    needed_page_count = (100*page_item->struct_size)/system_page_size + 1;

    meta_block_t *third_free_meta_block = allocate_free_data_block(page_item, 100 * page_item->struct_size);
    assert(third_free_meta_block != NULL);

    assert(page_item->first_page->next != NULL);
    assert(page_item->first_page->system_page_count == needed_page_count);

    free_data_blocks(first_free_meta_block);
    free_data_blocks(third_free_meta_block);

    assert(page_item->first_page == NULL);

    PRINT_SUCCESS(__func__);
}


int main()
{
    set_system_page_size();

    printf("running memtool tests...\n");

    test_page_item_registration();
    test_page_item_registration_for_few();
    test_page_item_registration_for_multiple();
    
    register_page_item("test_x", sizeof(test_x));
    register_page_item("test_y", sizeof(test_y));
    register_page_item("test_z", sizeof(test_z));

    test_free_data_block_allocation_small_size();
    test_free_data_block_allocation_medium_size();
    test_free_data_block_allocation_large_size();

    test_free_data_block_allocation_for_consecutive_times();
}
