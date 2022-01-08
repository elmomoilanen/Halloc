/*
 * APIs for doubly linked list `dll_node_t`.
*/
#ifndef __DLL__
#define __DLL__

#include <stdint.h>
#include <stddef.h>


typedef struct dll_node_ {
    struct dll_node_ *prev;
    struct dll_node_ *next;
} dll_node_t;

typedef struct dll_ {
    dll_node_t *head;
    uint32_t offset;
} dll_t;

typedef int16_t (*comp_func)(void *, void *);


#define GET_DLL_DATA(dll_node, offset) ((void *)((char *)dll_node - offset))

#define GET_ITEM_OFFSET(struct, item) ((size_t)(&((struct *)0)->item))

#define TRAVERSE_DLL_FORWARD_BEGIN(dll_node)        \
{                                                   \
    dll_node_t *_next_node = NULL;                  \
    for(; dll_node != NULL; dll_node = _next_node){ \
        _next_node = dll_node->next;

#define TRAVERSE_DLL_FORWARD_END(dll_node) }}

#define TRAVERSE_DLL_BACKWARD_BEGIN(dll_node)       \
{                                                   \
    dll_node_t *_prev_node = NULL;                  \
    for(; dll_node != NULL; dll_node = _prev_node){ \
        _prev_node = dll_node->prev;

#define TRAVERSE_DLL_BACKWARD_END(dll_node) }}


void _init_node(dll_node_t *node);
void _push_node(dll_t *dll, dll_node_t *node);
void _append_node(dll_t *dll, dll_node_t *node);

void _add_node_after(dll_node_t *node, dll_node_t *new_node);
void _add_node_before(dll_node_t *node, dll_node_t *new_node);

void _unlink_node(dll_node_t *node);
void _remove_node(dll_t *dll, dll_node_t *node);

void _add_to_priority_queue(dll_node_t *head, dll_node_t *new_node, uint32_t node_offset, comp_func func);

dll_node_t* _msort(dll_node_t *head, comp_func func);


#endif /* __DLL__ */
