/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_SQRT_LIST_H
#define _LINUX_SQRT_LIST_H

#include <linux/types.h>
#include <linux/stddef.h>
#include <linux/poison.h>
#include <linux/const.h>
#include <linux/kernel.h>

#define SQRT_LIST_POISON1 ((void *) 0x100 + POISON_POINTER_DELTA)
#define SQRT_LIST_POISON2 ((void *) 0x200 + POISON_POINTER_DELTA)
#define SQRT_LIST_POISON3 ((void *) 0x300 + POISON_POINTER_DELTA)
#define SQRT_LIST_POISON4 ((void *) 0x400 + POISON_POINTER_DELTA)
#define SQRT_LIST_POISON5 ((void *) 0x500 + POISON_POINTER_DELTA)

struct sqrt_list_head {
	struct sqrt_list_head *next, *prev, *head, *bucket;
	struct sqrt_list_head *b_next, *b_prev; // only for bucket(and also head)
	unsigned long b_next_gap; // only for bucket(and also head)
	unsigned long h_base_gap, h_size; // only for head
};

#define SQRT_LIST_HEAD_INIT(name) { &(name), &(name), &(name), &(name), NULL, NULL, 0, 2, 0 }

#define SQRT_LIST_HEAD(name) \
	struct sqrt_list_head name = SQRT_LIST_HEAD_INIT(name)

static inline void INIT_SQRT_LIST_HEAD(struct sqrt_list_head *list)
{
	WRITE_ONCE(list->next, list);
	list->prev = list;
	list->head = list;
	list->bucket = list;
	list->b_next = NULL;
	list->b_prev = NULL;
	list->b_next_gap = 0;
	list->h_base_gap = 2;
	list->h_size = 0;
}

#ifdef CONFIG_DEBUG_SQRT_LIST
extern bool __sqrt_list_add_valid(struct sqrt_list_head *new,
			      struct sqrt_list_head *prev,
			      struct sqrt_list_head *next);
extern bool __sqrt_list_del_entry_valid(struct sqrt_list_head *entry);
#else
static inline bool __sqrt_list_add_valid(struct sqrt_list_head *new,
				struct sqrt_list_head *prev,
				struct sqrt_list_head *next)
{
	return true;
}
static inline bool __sqrt_list_del_entry_valid(struct sqrt_list_head *entry)
{
	return true;
}
#endif

static inline unsigned long ___sqrt_list_calc_proper_gap(struct sqrt_list_head *head)
{
	unsigned long gap = int_sqrt(head->h_size);
	return (gap > 2) ? gap : 2;
}

// amortized O(sqrt(n))
// O(n) cost, once per O(sqrt(n)) times = O(n/sqrt(n)) = O(sqrt(n))
static inline void ___sqrt_list_arrange_buckets(struct sqrt_list_head *head)
{
	struct sqrt_list_head *pos, *tmp;
	unsigned long passed;
	unsigned long gap = ___sqrt_list_calc_proper_gap(head);

	if (((gap > head->h_base_gap) ? gap - head->h_base_gap : head->h_base_gap - gap) < 2) {
		return;
	}

	head->bucket = head;
	head->b_next = NULL;
	head->b_prev = NULL;
	head->b_next_gap = 0;
	head->h_base_gap = gap;

	for (pos = head->next, tmp = head, passed = 0; pos != head; pos = pos->next) {
		pos->bucket = tmp;

		if (++passed == gap) {
			passed = 0;
			pos->bucket = pos;
			tmp->b_next = pos;
			pos->b_prev = tmp;
			tmp->b_next_gap = gap;
			tmp = pos;
		}
	}
}

static inline void __sqrt_list_assign_to_bucket(struct sqrt_list_head *entry, struct sqrt_list_head *bucket)
{
	entry->head = bucket->head;
	entry->bucket = bucket;
	bucket->b_next_gap++;
	bucket->head->h_size++;
	___sqrt_list_arrange_buckets(bucket->head);
}

static inline void __sqrt_list_remove_from_bucket(struct sqrt_list_head *entry, struct sqrt_list_head *bucket)
{
	bucket->b_next_gap--;
	bucket->head->h_size--;

	if (bucket == entry) {
		if (bucket->next == bucket->head) {
			bucket->b_prev->b_next = NULL;
		} else if (bucket->next == bucket->b_next) {
			bucket->b_prev->b_next = bucket->b_next;
			bucket->b_next->b_prev = bucket->b_prev;
		} else {
			struct sqrt_list_head *pos;
			struct sqrt_list_head *new_bucket = bucket->next;
			new_bucket->b_next_gap = 0;

			// O(sqrt(n)) - change bucket
			for (pos = new_bucket; pos && pos->bucket == bucket; pos = pos->next) {
				pos->bucket = new_bucket;
				new_bucket->b_next_gap++;
			}

			new_bucket->b_prev = bucket->b_prev;
			new_bucket->b_next = bucket->b_next;
			new_bucket->b_prev->b_next = new_bucket;

			if (new_bucket->b_next) {
				new_bucket->b_next->b_prev = new_bucket;
			}
		}
	}

	___sqrt_list_arrange_buckets(bucket->head);
}

static inline void __sqrt_list_add(struct sqrt_list_head *new,
			      struct sqrt_list_head *prev,
			      struct sqrt_list_head *next)
{
	if (!__sqrt_list_add_valid(new, prev, next))
		return;

	next->prev = new;
	new->next = next;
	new->prev = prev;
	WRITE_ONCE(prev->next, new);
	__sqrt_list_assign_to_bucket(new, prev->bucket);
}

static inline void sqrt_list_add(struct sqrt_list_head *new, struct sqrt_list_head *head)
{
	__sqrt_list_add(new, head, head->next);
}

static inline void sqrt_list_add_tail(struct sqrt_list_head *new, struct sqrt_list_head *head)
{
	__sqrt_list_add(new, head->prev, head);
}

static inline void __sqrt_list_del_entry(struct sqrt_list_head *entry)
{
	if (!__sqrt_list_del_entry_valid(entry))
		return;

	entry->next->prev = entry->prev;
	WRITE_ONCE(entry->prev->next, entry->next);
	__sqrt_list_remove_from_bucket(entry, entry->bucket);
}

static inline void sqrt_list_del(struct sqrt_list_head *entry)
{
	__sqrt_list_del_entry(entry);
	entry->next = SQRT_LIST_POISON1;
	entry->prev = SQRT_LIST_POISON2;
	entry->head = SQRT_LIST_POISON3;
	entry->bucket = SQRT_LIST_POISON4;
	entry->b_next = SQRT_LIST_POISON5;
}

static inline void sqrt_list_del_init(struct sqrt_list_head *entry)
{
	__sqrt_list_del_entry(entry);
	INIT_SQRT_LIST_HEAD(entry);
}

static inline void sqrt_list_move(struct sqrt_list_head *list, struct sqrt_list_head *head)
{
	__sqrt_list_del_entry(list);
	sqrt_list_add(list, head);
}

static inline void sqrt_list_move_tail(struct sqrt_list_head *list,
				  struct sqrt_list_head *head)
{
	__sqrt_list_del_entry(list);
	sqrt_list_add_tail(list, head);
}

static inline int sqrt_list_is_first(const struct sqrt_list_head *list,
					const struct sqrt_list_head *head)
{
	return list->prev == head;
}

static inline int sqrt_list_is_last(const struct sqrt_list_head *list,
				const struct sqrt_list_head *head)
{
	return list->next == head;
}

static inline int sqrt_list_empty(const struct sqrt_list_head *head)
{
	return READ_ONCE(head->next) == head;
}

static inline int sqrt_list_empty_careful(const struct sqrt_list_head *head)
{
	struct sqrt_list_head *next = head->next;
	return (next == head) && (next == head->prev);
}

static inline int sqrt_list_is_singular(const struct sqrt_list_head *head)
{
	return !sqrt_list_empty(head) && (head->next == head->prev);
}

static inline struct sqrt_list_head *sqrt_list_nth(struct sqrt_list_head *head, unsigned long n)
{
	struct sqrt_list_head *pos;

	if (n < 1 || n > head->h_size) {
		return NULL;
	}

	// O(sqrt(n)) - find bucket
	for (pos = head; n >= pos->b_next_gap && pos->b_next; n -= pos->b_next_gap, pos = pos->b_next);
	// O(sqrt(n)) - find offset
	for (; n--; pos = pos->next);
	return pos;
}

#define sqrt_list_entry(ptr, type, member) \
	container_of(ptr, type, member)

#define sqrt_list_first_entry(ptr, type, member) \
	sqrt_list_entry((ptr)->next, type, member)

#define sqrt_list_last_entry(ptr, type, member) \
	sqrt_list_entry((ptr)->prev, type, member)

#define sqrt_list_first_entry_or_null(ptr, type, member) ({ \
	struct sqrt_list_head *head__ = (ptr); \
	struct sqrt_list_head *pos__ = READ_ONCE(head__->next); \
	pos__ != head__ ? sqrt_list_entry(pos__, type, member) : NULL; \
})

#define sqrt_list_next_entry(pos, member) \
	sqrt_list_entry((pos)->member.next, typeof(*(pos)), member)

#define sqrt_list_prev_entry(pos, member) \
	sqrt_list_entry((pos)->member.prev, typeof(*(pos)), member)

#define sqrt_list_for_each(pos, head) \
	for (pos = (head)->next; pos != (head); pos = pos->next)

#define sqrt_list_for_each_safe(pos, n, head) \
	for (pos = (head)->next, n = pos->next; pos != (head); \
		pos = n, n = pos->next)

#define sqrt_list_entry_is_head(pos, head, member) \
	(&pos->member == (head))

#define sqrt_list_for_each_entry(pos, head, member) \
	for (pos = sqrt_list_first_entry(head, typeof(*pos), member); \
	     !sqrt_list_entry_is_head(pos, head, member); \
	     pos = sqrt_list_next_entry(pos, member))

#define sqrt_list_for_each_entry_safe(pos, n, head, member) \
	for (pos = sqrt_list_first_entry(head, typeof(*pos), member), \
		n = sqrt_list_next_entry(pos, member); \
	     !sqrt_list_entry_is_head(pos, head, member); \
	     pos = n, n = sqrt_list_next_entry(n, member))

#define sqrt_list_safe_reset_next(pos, n, member) \
	n = sqrt_list_next_entry(pos, member)

#endif
