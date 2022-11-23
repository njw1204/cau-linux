#include <linux/kernel.h>
#include <linux/ktime.h>
#include "tester.h"
#include "calclock.h"
#include "linux/list.h"
#include "linux/sqrt_list.h"

#define TEST_SIZE 100000

struct array_node {
    int data;
};

struct list_node {
    struct list_head entry;
    int data;
};

struct sqrt_list_node {
    struct sqrt_list_head entry;
    int data;
};

// random number between 0 and 2^31-1
static int rand(unsigned int *seed)
{
    unsigned int next = *seed;
    int result;

    next *= 1103515245;
    next += 12345;
    result = (unsigned int) (next / 65536) % 2048;

    next *= 1103515245;
    next += 12345;
    result <<= 10;
    result ^= (unsigned int) (next / 65536) % 1024;

    next *= 1103515245;
    next += 12345;
    result <<= 10;
    result ^= (unsigned int) (next / 65536) % 1024;

    *seed = next;

    return result;
}

void array_test(void)
{
    static struct array_node data[TEST_SIZE * 2];
    int i, j;
    int array_size = 0;
    unsigned int seed = 42;
    unsigned long long total_time_1 = 0, total_count_1 = 0;
    unsigned long long total_time_2 = 0, total_count_2 = 0;
    unsigned long long total_time_3 = 0, total_count_3 = 0;
    unsigned long long total_time_4 = 0, total_count_4 = 0;
    struct array_node *pos_node;
    struct timespec times[2];

    for (i = TEST_SIZE; i >= 1; i--) {
        getrawmonotonic(&times[0]);
        array_size++;

        for (j = array_size; j >= 2; j--) {
            data[j] = data[j - 1];
        }

        data[1].data = i;
        getrawmonotonic(&times[1]);
        calclock(times, &total_time_3, &total_count_3);
    }

    for (i = 0; i < TEST_SIZE; i++) {
        int target = rand(&seed) % TEST_SIZE + 1;
        getrawmonotonic(&times[0]);
        pos_node = &data[target];
        WARN_ON(pos_node->data != target);
        getrawmonotonic(&times[1]);
        calclock(times, &total_time_1, &total_count_1);
    }

    j = 0;
    getrawmonotonic(&times[0]);

    for (i = 1; i <= TEST_SIZE; i++) {
        pos_node = &data[i];
        WARN_ON(pos_node->data != ++j);
        getrawmonotonic(&times[1]);
        calclock(times, &total_time_2, &total_count_2);
        getrawmonotonic(&times[0]);
    }

    for (i = 0; i < TEST_SIZE; i++) {
        getrawmonotonic(&times[0]);
        array_size--;

        for (j = 1; j <= array_size; j++) {
            data[j] = data[j + 1];
        }

        getrawmonotonic(&times[1]);
        calclock(times, &total_time_4, &total_count_4);
    }

    printk("array_test     (random access)    : %10llu ns (%llu times)\n", total_time_1, total_count_1);
    printk("array_test     (sequential access): %10llu ns (%llu times)\n", total_time_2, total_count_2);
    printk("array_test     (insert)           : %10llu ns (%llu times)\n", total_time_3, total_count_3);
    printk("array_test     (delete)           : %10llu ns (%llu times)\n", total_time_4, total_count_4);
    printk("array_test     (total)            : \e[0;36m%10llu ns\e[0m\n", total_time_1 + total_time_2 + total_time_3 + total_time_4);
}

void list_test(void)
{
    static struct list_node data[TEST_SIZE];
    int i, j;
    unsigned int seed = 42;
    unsigned long long total_time_1 = 0, total_count_1 = 0;
    unsigned long long total_time_2 = 0, total_count_2 = 0;
    unsigned long long total_time_3 = 0, total_count_3 = 0;
    unsigned long long total_time_4 = 0, total_count_4 = 0;
    struct list_head *pos;
    struct list_head head;
    struct timespec times[2];

    INIT_LIST_HEAD(&head);

    for (i = TEST_SIZE; i >= 1; i--) {
        struct list_node *new = &data[i - 1];
        new->data = i;
        getrawmonotonic(&times[0]);
        list_add(&new->entry, &head);
        getrawmonotonic(&times[1]);
        calclock(times, &total_time_3, &total_count_3);
    }

    for (i = 0; i < TEST_SIZE; i++) {
        int target = rand(&seed) % TEST_SIZE + 1;
        j = 0;
        getrawmonotonic(&times[0]);

        list_for_each(pos, &head) {
            if (++j == target) break;
        }

        WARN_ON(list_entry(pos, struct list_node, entry)->data != target);
        getrawmonotonic(&times[1]);
        calclock(times, &total_time_1, &total_count_1);
    }

    i = 0;
    getrawmonotonic(&times[0]);

    list_for_each(pos, &head) {
        WARN_ON(list_entry(pos, struct list_node, entry)->data != ++i);
        getrawmonotonic(&times[1]);
        calclock(times, &total_time_2, &total_count_2);
        getrawmonotonic(&times[0]);
    }

    i = 0;

    while (!list_empty(&head)) {
        WARN_ON(list_entry(head.next, struct list_node, entry)->data != ++i);
        getrawmonotonic(&times[0]);
        list_del(head.next);
        getrawmonotonic(&times[1]);
        calclock(times, &total_time_4, &total_count_4);
    }

    printk("list_test      (random access)    : %10llu ns (%llu times)\n", total_time_1, total_count_1);
    printk("list_test      (sequential access): %10llu ns (%llu times)\n", total_time_2, total_count_2);
    printk("list_test      (insert)           : %10llu ns (%llu times)\n", total_time_3, total_count_3);
    printk("list_test      (delete)           : %10llu ns (%llu times)\n", total_time_4, total_count_4);
    printk("list_test      (total)            : \e[0;36m%10llu ns\e[0m\n", total_time_1 + total_time_2 + total_time_3 + total_time_4);
}

void sqrt_list_test(void)
{
    static struct sqrt_list_node data[TEST_SIZE];
    int i;
    unsigned int seed = 42;
    unsigned long long total_time_1 = 0, total_count_1 = 0;
    unsigned long long total_time_2 = 0, total_count_2 = 0;
    unsigned long long total_time_3 = 0, total_count_3 = 0;
    unsigned long long total_time_4 = 0, total_count_4 = 0;
    struct sqrt_list_head *pos;
    struct sqrt_list_head head;
    struct timespec times[2];

    INIT_SQRT_LIST_HEAD(&head);

    for (i = TEST_SIZE; i >= 1; i--) {
        struct sqrt_list_node *new = &data[i - 1];
        new->data = i;
        getrawmonotonic(&times[0]);
        sqrt_list_add(&new->entry, &head);
        getrawmonotonic(&times[1]);
        calclock(times, &total_time_3, &total_count_3);
    }

    for (i = 0; i < TEST_SIZE; i++) {
        int target = rand(&seed) % TEST_SIZE + 1;
        getrawmonotonic(&times[0]);
        pos = sqrt_list_nth(&head, target);
        WARN_ON(list_entry(pos, struct sqrt_list_node, entry)->data != target);
        getrawmonotonic(&times[1]);
        calclock(times, &total_time_1, &total_count_1);
    }

    i = 0;
    getrawmonotonic(&times[0]);

    sqrt_list_for_each(pos, &head) {
        WARN_ON(list_entry(pos, struct sqrt_list_node, entry)->data != ++i);
        getrawmonotonic(&times[1]);
        calclock(times, &total_time_2, &total_count_2);
        getrawmonotonic(&times[0]);
    }

    i = 0;

    while (!sqrt_list_empty(&head)) {
        WARN_ON(list_entry(head.next, struct sqrt_list_node, entry)->data != ++i);
        getrawmonotonic(&times[0]);
        sqrt_list_del(head.next);
        getrawmonotonic(&times[1]);
        calclock(times, &total_time_4, &total_count_4);
    }

    printk("sqrt_list_test (random access)    : %10llu ns (%llu times)\n", total_time_1, total_count_1);
    printk("sqrt_list_test (sequential access): %10llu ns (%llu times)\n", total_time_2, total_count_2);
    printk("sqrt_list_test (insert)           : %10llu ns (%llu times)\n", total_time_3, total_count_3);
    printk("sqrt_list_test (delete)           : %10llu ns (%llu times)\n", total_time_4, total_count_4);
    printk("sqrt_list_test (total)            : \e[0;36m%10llu ns\e[0m\n", total_time_1 + total_time_2 + total_time_3 + total_time_4);
}
