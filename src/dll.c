#include <stdlib.h>
#include <stdio.h>

#include "dll.h"


void _init_node(dll_node_t *node) {
    node->prev = NULL;
    node->next = NULL;
}

void _push_node(dll_t *dll, dll_node_t *node) {
    node->next = dll->head;
    if(node->next != NULL) node->next->prev = node;
    dll->head = node;
}

void _append_node(dll_t *dll, dll_node_t *node) {
    if(dll->head == NULL) {
        dll->head = node;
        return;
    }

    dll_node_t *_node = dll->head;
    dll_node_t *active_node = NULL;

    TRAVERSE_DLL_FORWARD_BEGIN(_node)
    {active_node = _node;}
    TRAVERSE_DLL_FORWARD_END(_node);

    active_node->next = node;
    node->prev = active_node;
}

void _add_node_after(dll_node_t *node, dll_node_t *new_node) {
    new_node->next = node->next;
    if(new_node->next != NULL) new_node->next->prev = new_node;
    node->next = new_node;
    new_node->prev = node;
}

void _add_node_before(dll_node_t *node, dll_node_t *new_node) {
    // not safe for head node, use push in that case
    new_node->next = node;
    if(new_node->next->prev != NULL) {
        new_node->prev = new_node->next->prev;
        new_node->prev->next = new_node;
    }
    new_node->next->prev = new_node;
}

void _unlink_node(dll_node_t *node) {
    // not safe for head node, use remove_node in that case
    if(node->prev == NULL) {
        if(node->next) {
            node->next->prev = NULL;
            node->next = NULL;
        }
    } else {
        if(node->next) {
            node->next->prev = node->prev;
            node->prev->next = node->next;
            node->prev = NULL;
            node->next = NULL;
        } else {
            node->prev->next = NULL;
            node->prev = NULL;
        }
    }
}

void _remove_node(dll_t *dll, dll_node_t *node) {
    if(dll->head == node) dll->head = node->next;
    _unlink_node(node);
}


void _add_to_priority_queue(
    dll_node_t *head,
    dll_node_t *new_node,
    uint32_t node_offset,
    comp_func func) {
    // head->next is the first real node
    if(head->next == NULL && head->prev == NULL) {
        _add_node_after(head, new_node);
        return;
    }

    if(head->next == NULL) {
        // should never land here, if program logic ok
        fprintf(stderr,
        "%s(): priority queue head next node NULL but prev node not NULL.\n",
        __func__);

        exit(EXIT_FAILURE);
    }

    if(func(GET_DLL_DATA(new_node, node_offset), GET_DLL_DATA(head->next, node_offset)) < 0) {
        // negative return value, lhs value is greater than rhs
        _add_node_after(head, new_node);
        return;
    }

    dll_node_t *node = head->next;
    dll_node_t *active_node = NULL;

    TRAVERSE_DLL_FORWARD_BEGIN(node)
    {
        if(func(GET_DLL_DATA(new_node, node_offset), GET_DLL_DATA(node, node_offset)) < 0) {
            // negative return value, lhs value is greater than rhs
            _add_node_before(node, new_node);
            return;
        }
        active_node = node;
    }
    TRAVERSE_DLL_FORWARD_END(node);

    _add_node_after(active_node, new_node);
}


static dll_node_t* _split_dll(dll_node_t *head) {
    dll_node_t *fast = head, *slow = head;

    while(fast->next && fast->next->next) {
        slow = slow->next;
        fast = fast->next->next;
    }
    dll_node_t *node = slow->next;
    slow->next = NULL;

    return node;
}

static dll_node_t* _merge(dll_node_t *left, dll_node_t *right, comp_func func) {
    if(left == NULL) return right;
    if(right == NULL) return left;

    /*
    Function pointer `func` must be capable of sorting nodes
    of the linked list such that it returns a positive integer
    if the left node is smaller than right node and negative integer
    if the right node is smaller.
    */
    if(func(left, right) > 0) {
        left->next = _merge(left->next, right, func);
        left->next->prev = left;
        left->prev = NULL;
        return left;
    }

    right->next = _merge(left, right->next, func);
    right->next->prev = right;
    right->prev = NULL;
    return right;
}

dll_node_t* _msort(dll_node_t *head, comp_func func) {
    if(head == NULL || head->next == NULL) return head;

    dll_node_t *tail = _split_dll(head);

    head = _msort(head, func);
    tail = _msort(tail, func);

    return _merge(head, tail, func);
}
