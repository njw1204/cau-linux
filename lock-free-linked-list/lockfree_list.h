/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LOCKFREE_LIST_H
#define _LOCKFREE_LIST_H

#include <linux/types.h>
#include <linux/stddef.h>
#include <linux/poison.h>
#include <linux/const.h>
#include <linux/kernel.h>

static inline void INIT_LF_LIST_HEAD(struct list_head *list)
{
    list->next = NULL;
    list->prev = list;
}

/**
 * list_add_tail - add a new entry
 * @new: new entry to be added
 * @head: list head to add it before
 *
 * Insert a new entry before the specified head.
 * This is useful for implementing queues.
 */
static inline void 
lf_list_add_tail(struct list_head *entry, struct list_head *head)
{
    entry->prev = __sync_lock_test_and_set(&head->prev, entry);

    if (entry->prev == NULL)
        head = entry;
    else
        entry->prev->next = entry;
}

/*
 * Delete a list entry by making the prev/next entries
 * point to each other.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static inline void 
__gc_list_del(struct list_head *prev, struct list_head *next, struct list_head *head)
{
    if (next)
        next->prev = prev;
    else
        head->prev = prev;
    
    prev->next = next; 
}

static inline void 
gc_list_del(struct list_head *entry, struct list_head *head)
{
    __gc_list_del(entry->prev, entry->next, head);
}

#endif
