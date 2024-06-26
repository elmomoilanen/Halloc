#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "common.h"
#include "dll.h"

typedef struct {
    u64 x, x2;
    char n[32];
    u64 x3;
    dll_node_t node;
} test_type_t;


static int16_t test_type_comparer(void *left, void *right) {
    dll_node_t *left_node = left;
    dll_node_t *right_node = right;

    uint32_t data_offset = GET_ITEM_OFFSET(test_type_t, node);

    u64 x_left = ((test_type_t *)GET_DLL_DATA(left_node, data_offset))->x;
    u64 x_right = ((test_type_t *)GET_DLL_DATA(right_node, data_offset))->x;

    if ( x_left >= x_right ) return -1;
    return 1;
}

static int16_t test_type_comparer_(void *left, void *right) {
    if ( ((test_type_t *)left)->x > ((test_type_t *)right)->x ) return -1;
    return 1;
}

static void test_offset_macro() {
    assert(GET_ITEM_OFFSET(test_type_t, x) == offsetof(test_type_t, x));
    assert(GET_ITEM_OFFSET(test_type_t, x2) == offsetof(test_type_t, x2));
    assert(GET_ITEM_OFFSET(test_type_t, n) == offsetof(test_type_t, n));
    assert(GET_ITEM_OFFSET(test_type_t, x3) == offsetof(test_type_t, x3));
    assert(GET_ITEM_OFFSET(test_type_t, node) == offsetof(test_type_t, node));

    PRINT_SUCCESS(__func__);
}

static void test_data_offset_macro() {
    test_type_t test_struct = {0};
    test_struct.x = 1;

    _init_node(&test_struct.node);
    dll_t dll = {
        .head=&test_struct.node,
        .offset=GET_ITEM_OFFSET(test_type_t, node)
    };

    test_type_t *data = GET_DLL_DATA(dll.head, dll.offset);

    assert(data->x == 1);

    PRINT_SUCCESS(__func__);
}

static void test_node_pushing() {
    test_type_t test_struct = {0}, test_struct2 = {0};
    test_struct.x = 1;
    test_struct2.x = 2;

    _init_node(&test_struct.node);
    _init_node(&test_struct2.node);

    dll_t dll = {
        .head=NULL,
        .offset=GET_ITEM_OFFSET(test_type_t, node)
    };

    _push_node(&dll, &test_struct.node);
    _push_node(&dll, &test_struct2.node);

    assert(dll.head->next != NULL);
    assert(dll.head->prev == NULL);

    assert(((test_type_t *)GET_DLL_DATA(dll.head, dll.offset))->x == 2);

    PRINT_SUCCESS(__func__);
}

static void test_traverse_forward_macro() {
    u32 n_structs = 3, node_counter = 0;
    test_type_t *test_structs = calloc(n_structs, sizeof *test_structs);

    dll_t *dll = calloc(1, sizeof *dll);

    for (u32 j=0; j<n_structs; ++j) _push_node(dll, &test_structs[j].node);

    dll_node_t *node = dll->head;

    TRAVERSE_DLL_FORWARD_BEGIN(node)
    {node_counter += 1;}
    TRAVERSE_DLL_FORWARD_END(node);

    assert(node_counter == n_structs);
    assert(node == NULL);

    free(dll);
    free(test_structs);

    PRINT_SUCCESS(__func__);
}

static void test_data_offset_macro_multiple_nodes() {
    u32 const structs = 3;
    test_type_t *test_structs = calloc(structs, sizeof *test_structs); 

    dll_t dll = {
        .head=NULL,
        .offset=GET_ITEM_OFFSET(test_type_t, node)
    };

    for (u32 j=0; j<structs; ++j)
    {
        _init_node(&test_structs[j].node);
        _push_node(&dll, &test_structs[j].node);
    }

    dll_node_t *node = dll.head;
    u32 j = 0;

    TRAVERSE_DLL_FORWARD_BEGIN(node)
    {
        i32 i = structs-1-j;
        assert(i >= 0);
        assert( ((test_type_t *)GET_DLL_DATA(node, dll.offset)) == &test_structs[i] );

        ++j;
    }
    TRAVERSE_DLL_FORWARD_END(node);

    free(test_structs);

    PRINT_SUCCESS(__func__);
}

static void test_traverse_backward_macro() {
    u32 n_structs = 5, node_counter = 0;
    test_type_t *test_structs = calloc(n_structs, sizeof *test_structs);

    dll_t *dll = calloc(1, sizeof *dll);
    dll->offset = GET_ITEM_OFFSET(test_type_t, node);

    for (u32 j=0; j<n_structs; ++j) _push_node(dll, &test_structs[j].node);

    dll_node_t *node = dll->head;
    dll_node_t *active_node = NULL;

    TRAVERSE_DLL_FORWARD_BEGIN(node)
    {active_node = node;}
    TRAVERSE_DLL_FORWARD_END(node);

    assert(active_node != NULL);
    assert(active_node->prev != NULL);
    assert(active_node->next == NULL);

    node = active_node;

    TRAVERSE_DLL_BACKWARD_BEGIN(node)
    {
        node_counter += 1;
        active_node = node;
    }
    TRAVERSE_DLL_BACKWARD_END(node);

    assert(node_counter == n_structs);

    assert(node == NULL);
    assert(active_node == dll->head);

    free(dll);
    free(test_structs);

    PRINT_SUCCESS(__func__);
}

static void test_node_appending() {
    u32 const n_structs = 4;
    test_type_t *test_structs = calloc(n_structs, sizeof *test_structs);

    dll_t dll = {
        .head=NULL,
        .offset=GET_ITEM_OFFSET(test_type_t, node)
    };

    for (u32 j=0; j<n_structs; ++j)
    {
        test_structs[j].x = j + 1;
        _append_node(&dll, &test_structs[j].node);
    }

    u32 node_counter = 0;
    dll_node_t *node = dll.head;

    TRAVERSE_DLL_FORWARD_BEGIN(node)
    {
        node_counter += 1;
        assert( ((test_type_t *)GET_DLL_DATA(node, dll.offset))->x == node_counter );
    }
    TRAVERSE_DLL_FORWARD_END(node);

    assert(node_counter == n_structs);

    free(test_structs);

    PRINT_SUCCESS(__func__);
}

static void test_adding_node_after() {
    test_type_t test_struct = {0};
    test_struct.x = 1;
    _init_node(&test_struct.node);

    dll_t dll = {
        .head=&test_struct.node,
        .offset=GET_ITEM_OFFSET(test_type_t, node)
    };

    test_type_t test_struct2 = {0};
    test_struct2.x = 2;
    _init_node(&test_struct2.node);

    _add_node_after(dll.head, &test_struct2.node);

    test_type_t test_struct3 = {0};
    test_struct3.x = 3;
    _init_node(&test_struct3.node);

    _add_node_after(dll.head, &test_struct3.node);

    u32 n_structs = 3;
    u32 correct_index_order[] = {1,3,2};

    u32 j = 0;
    dll_node_t *node = dll.head;

    TRAVERSE_DLL_FORWARD_BEGIN(node)
    {
        assert(j < n_structs);
        assert( ((test_type_t *)GET_DLL_DATA(node, dll.offset))->x == correct_index_order[j] );
        j++;
    }
    TRAVERSE_DLL_FORWARD_END(node);

    PRINT_SUCCESS(__func__);
}

static void test_adding_node_before() {
    test_type_t ts = {0};
    ts.x2 = 5;
    _init_node(&ts.node);

    dll_t dll = {
        .head=&ts.node,
        .offset=GET_ITEM_OFFSET(test_type_t, node)
    };

    test_type_t test_structs[2] = {{0}};
    test_structs[0].x2 = 7;
    test_structs[1].x2 = 11;
    _init_node(&test_structs[0].node);
    _init_node(&test_structs[1].node);

    _add_node_after(dll.head, &test_structs[0].node);
    _add_node_before(dll.head->next, &test_structs[1].node);

    u32 correct_x2_order[] = {5, 11, 7};
    u32 j = 0;
    dll_node_t *node = dll.head;

    TRAVERSE_DLL_FORWARD_BEGIN(node)
    {
        assert( ((test_type_t *)GET_DLL_DATA(node, dll.offset))->x2 == correct_x2_order[j] );
        j += 1;
    }
    TRAVERSE_DLL_FORWARD_END(node);

    PRINT_SUCCESS(__func__);
}

static void test_unlinking_node() {
    u32 const n_structs = 3;
    test_type_t *test_structs = calloc(n_structs, sizeof *test_structs);

    dll_t dll = {
        .head=NULL,
        .offset=GET_ITEM_OFFSET(test_type_t, node)
    };

    for (u32 j=0; j<n_structs; ++j) _push_node(&dll, &test_structs[j].node);

    dll_node_t *node = dll.head;
    u32 j = 0;

    TRAVERSE_DLL_FORWARD_BEGIN(node)
    {
        if (j == 1) _unlink_node(node);
        j += 1;
    }
    TRAVERSE_DLL_FORWARD_END(node);

    assert( (test_type_t *)GET_DLL_DATA(dll.head, dll.offset) == &test_structs[2] );
    assert( (test_type_t *)GET_DLL_DATA(dll.head->next, dll.offset) == &test_structs[0]);

    assert(dll.head->next->next == NULL);

    free(test_structs);

    PRINT_SUCCESS(__func__);
}

static void test_removal_of_node() {
    u32 const n_structs = 5;
    test_type_t *test_structs = calloc(n_structs, sizeof *test_structs);

    dll_t dll = {
        .head=NULL,
        .offset=GET_ITEM_OFFSET(test_type_t, node)
    };

    for (u32 j=0; j<n_structs; ++j)
    {
        test_structs[j].x = j + 1;
        _push_node(&dll, &test_structs[j].node);
    }

    u32 j = 0;
    dll_node_t *node = dll.head;

    TRAVERSE_DLL_FORWARD_BEGIN(node)
    {
        if (j == 0 || j == 2) _remove_node(&dll, node);
        ++j;
    }
    TRAVERSE_DLL_FORWARD_END(node);

    assert( (test_type_t *)GET_DLL_DATA(dll.head, dll.offset) == &test_structs[3] );
    assert( ((test_type_t *)GET_DLL_DATA(dll.head, dll.offset))->x == 4 );

    assert( (test_type_t *)GET_DLL_DATA(dll.head->next, dll.offset) == &test_structs[1] );
    assert( (test_type_t *)GET_DLL_DATA(dll.head->next->next, dll.offset) == &test_structs[0] );

    assert(dll.head->next->next->next == NULL);

    free(test_structs);

    PRINT_SUCCESS(__func__);
}

static void test_adding_and_removal_of_nodes() {
    u32 const n_structs = 10;
    test_type_t *test_structs = calloc(n_structs, sizeof *test_structs);

    dll_t dll = {
        .head=NULL,
        .offset=GET_ITEM_OFFSET(test_type_t, node)
    };

    for (u32 j=0; j<n_structs; ++j)
    {
        test_structs[j].x = j;
        _push_node(&dll, &test_structs[j].node);
    }

    dll_node_t *node = dll.head;
    u32 structs_left = n_structs;

    TRAVERSE_DLL_FORWARD_BEGIN(node)
    {
        if ( ((test_type_t *)GET_DLL_DATA(node, dll.offset))->x % 3 == 0 )
        {
            _remove_node(&dll, node);
            structs_left -= 1;
        }
    }
    TRAVERSE_DLL_FORWARD_END(node);

    u32 correct_index_order[] = {8,7,5,4,2,1};
    u32 j = 0;

    assert(structs_left == sizeof(correct_index_order)/sizeof(correct_index_order[0]));

    node = dll.head;

    TRAVERSE_DLL_FORWARD_BEGIN(node)
    {
        assert(j < structs_left);
        assert( ((test_type_t *)GET_DLL_DATA(node, dll.offset))->x == correct_index_order[j] );
        j++;
    }
    TRAVERSE_DLL_FORWARD_END(node);

    free(test_structs);

    PRINT_SUCCESS(__func__);
}

static void test_dll_sorting() {
    u32 const n_structs = 7;
    test_type_t *test_structs = calloc(n_structs, sizeof *test_structs);

    dll_t dll = {
        .head=NULL,
        .offset=GET_ITEM_OFFSET(test_type_t, node)
    };

    u32 x_values[] = {17,11,2,3,7,23,2};
    u32 x_values_correct_order[] = {2,2,3,7,11,17,23};

    assert(n_structs == sizeof(x_values)/sizeof(x_values[0]));

    for (u32 j=0; j<n_structs; ++j)
    {
        test_structs[j].x = x_values[j];
        _push_node(&dll, &test_structs[j].node);
    }

    comp_func func = &test_type_comparer;

    dll_node_t *new_head = _msort(dll.head, func);

    u32 node_counter = 0;
    dll_node_t *node = new_head;

    TRAVERSE_DLL_FORWARD_BEGIN(node)
    {
        u64 x_val = ((test_type_t *)GET_DLL_DATA(node, dll.offset))->x;
        assert(x_val == x_values_correct_order[node_counter]);
        ++node_counter;
    }
    TRAVERSE_DLL_FORWARD_END(node);

    assert(node_counter == n_structs);

    free(test_structs);

    PRINT_SUCCESS(__func__);
}

static void test_priority_queue() {
    test_type_t tstructs[4] = {{0}};
    tstructs[0].x = 2;
    tstructs[1].x = 11;
    tstructs[2].x = 5;
    tstructs[3].x = 1;

    for (u32 j=0; j<4; ++j) _init_node(&tstructs[j].node);

    uint32_t offset = GET_ITEM_OFFSET(test_type_t, node);

    test_type_t phead = {0};
    _init_node(&phead.node);

    comp_func func = &test_type_comparer_;

    _add_to_priority_queue(&phead.node, &tstructs[0].node, offset, func);
    _add_to_priority_queue(&phead.node, &tstructs[1].node, offset, func);
    _add_to_priority_queue(&phead.node, &tstructs[2].node, offset, func);
    _add_to_priority_queue(&phead.node, &tstructs[3].node, offset, func);

    u32 correct_order_x[] = {11, 5, 2, 1};
    u32 i = 0;
    dll_node_t *node = &phead.node;

    TRAVERSE_DLL_FORWARD_BEGIN(node)
    {
        if (i == 0) continue;
        assert( ((test_type_t *)GET_DLL_DATA(node, offset))->x == correct_order_x[i] );
        ++i;
    }
    TRAVERSE_DLL_FORWARD_END(node);

    PRINT_SUCCESS(__func__);
}

test_func dll_tests[] = {
    {"offset_macro", test_offset_macro},
    {"data_offset_macro", test_data_offset_macro},
    {"node_pushing", test_node_pushing},
    {"traverse_forward_macro", test_traverse_forward_macro},
    {"data_offset_macro_multiple_nodes", test_data_offset_macro_multiple_nodes},
    {"traverse_backward_macro", test_traverse_backward_macro},
    {"node_appending", test_node_appending},
    {"adding_node_after", test_adding_node_after},
    {"adding_node_before", test_adding_node_before},
    {"unlinking_node", test_unlinking_node},
    {"removal_of_node", test_removal_of_node},
    {"adding_and_removal_of_nodes", test_adding_and_removal_of_nodes},
    {"dll_sorting", test_dll_sorting},
    {"priority_queue", test_priority_queue},
    {NULL, NULL},
};
