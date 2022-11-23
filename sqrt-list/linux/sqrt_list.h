/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_SQRT_LIST_H
#define _LINUX_SQRT_LIST_H

#include <linux/types.h>
#include <linux/stddef.h>
#include <linux/poison.h>
#include <linux/const.h>
#include <linux/kernel.h>

struct sqrt_list_head {
	struct sqrt_list_head *next, *prev;
};

#define SQRT_LIST_HEAD_INIT(name) { &(name), &(name) }

#define SQRT_LIST_HEAD(name) \
	struct sqrt_list_head name = SQRT_LIST_HEAD_INIT(name)

static inline void INIT_SQRT_LIST_HEAD(struct sqrt_list_head *list)
{
	WRITE_ONCE(list->next, list);
	list->prev = list;
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
}

static inline void sqrt_list_add(struct sqrt_list_head *new, struct sqrt_list_head *head)
{
	__sqrt_list_add(new, head, head->next);
}

static inline void sqrt_list_add_tail(struct sqrt_list_head *new, struct sqrt_list_head *head)
{
	__sqrt_list_add(new, head->prev, head);
}

static inline void __sqrt_list_del(struct sqrt_list_head *prev, struct sqrt_list_head *next)
{
	next->prev = prev;
	WRITE_ONCE(prev->next, next);
}

static inline void __sqrt_list_del_entry(struct sqrt_list_head *entry)
{
	if (!__sqrt_list_del_entry_valid(entry))
		return;

	__sqrt_list_del(entry->prev, entry->next);
}

static inline void sqrt_list_del(struct sqrt_list_head *entry)
{
	__sqrt_list_del_entry(entry);
	entry->next = LIST_POISON1;
	entry->prev = LIST_POISON2;
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
