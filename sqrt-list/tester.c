#include <linux/kernel.h>
#include <linux/ktime.h>
#include <linux/xarray.h>
#include "tester.h"
#include "calclock.h"
#include "linux/list.h"
#include "linux/sqrt_list.h"

#define TEST_SIZE 20000

struct xarray_node {
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

void xarray_test(void)
{
    static struct xarray_node data[TEST_SIZE];
    int i, j;
    unsigned int seed = 42;
    unsigned long long total_time[10] = {0};
    unsigned long long total_count[10] = {0};
    unsigned long pos;
    struct xarray_node *pos_node;
    struct xarray arr;
    int arr_size = 0;
    struct timespec times[2];

    xa_init(&arr);

    for (i = 0; i < TEST_SIZE; i++) {
        int target = rand(&seed) % (i + 1);
        getrawmonotonic(&times[0]);
        arr_size++;

        for (j = arr_size - 1; j > target; j--) {
            xa_store(&arr, j, xa_load(&arr, j - 1), GFP_KERNEL);
        }

        xa_store(&arr, target, &data[target], GFP_KERNEL);
        getrawmonotonic(&times[1]);
        calclock(times, &total_time[0], &total_count[0]);
    }

    for (i = 0; i < TEST_SIZE; i++) {
        data[i].data = i;
        xa_store(&arr, i, &data[i], GFP_KERNEL);
    }

    for (i = 0; i < TEST_SIZE; i++) {
        int target = rand(&seed) % TEST_SIZE;
        getrawmonotonic(&times[0]);
        pos_node = xa_load(&arr, target);
        WARN_ON(pos_node->data != target);
        getrawmonotonic(&times[1]);
        calclock(times, &total_time[1], &total_count[1]);
    }

    i = 0;
    getrawmonotonic(&times[0]);

    xa_for_each(&arr, pos, pos_node) {
        WARN_ON(pos_node->data != i++);
        getrawmonotonic(&times[1]);
        calclock(times, &total_time[2], &total_count[2]);
        getrawmonotonic(&times[0]);
    }

    for (i = TEST_SIZE; i >= 1; i--) {
        int target = rand(&seed) % i;
        getrawmonotonic(&times[0]);
        arr_size--;

        for (j = target; j < arr_size; j++) {
            xa_store(&arr, j, xa_load(&arr, j + 1), GFP_KERNEL);
        }

        xa_erase(&arr, arr_size);
        getrawmonotonic(&times[1]);
        calclock(times, &total_time[3], &total_count[3]);
    }

    xa_destroy(&arr);
    printk("xarray_test    (random access)    : %5llu.%02llu ms (%llu times)\n", total_time[1] / MILLION, total_time[1] * 100 / MILLION % 100, total_count[1]);
    printk("xarray_test    (sequential access): %5llu.%02llu ms (%llu times)\n", total_time[2] / MILLION, total_time[2] * 100 / MILLION % 100, total_count[2]);
    printk("xarray_test    (insert)           : %5llu.%02llu ms (%llu times)\n", total_time[0] / MILLION, total_time[0] * 100 / MILLION % 100, total_count[0]);
    printk("xarray_test    (delete)           : %5llu.%02llu ms (%llu times)\n", total_time[3] / MILLION, total_time[3] * 100 / MILLION % 100, total_count[3]);
    printk("xarray_test    (total)            : \e[0;36m%5llu.%02llu ms\e[0m\n", (total_time[0] + total_time[1] + total_time[2] + total_time[3]) / MILLION, (total_time[0] + total_time[1] + total_time[2] + total_time[3]) * 100 / MILLION % 100);
}

void list_test(void)
{
    static struct list_node data[TEST_SIZE];
    int i, j;
    unsigned int seed = 42;
    unsigned long long total_time[10] = {0};
    unsigned long long total_count[10] = {0};
    struct list_head *pos;
    struct list_head head;
    struct timespec times[2];

    INIT_LIST_HEAD(&head);

    for (i = 0; i < TEST_SIZE; i++) {
        struct list_node *new = &data[i];
        int target = rand(&seed) % (i + 1);
        pos = &head;

        if (target > 0) {
            j = 0;

            list_for_each(pos, &head) {
                if (++j == target) break;
            }
        }

        getrawmonotonic(&times[0]);
        list_add(&new->entry, pos);
        getrawmonotonic(&times[1]);
        calclock(times, &total_time[0], &total_count[0]);
    }

    i = 0;

    list_for_each(pos, &head) {
        list_entry(pos, struct list_node, entry)->data = ++i;
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
        calclock(times, &total_time[1], &total_count[1]);
    }

    i = 0;
    getrawmonotonic(&times[0]);

    list_for_each(pos, &head) {
        WARN_ON(list_entry(pos, struct list_node, entry)->data != ++i);
        getrawmonotonic(&times[1]);
        calclock(times, &total_time[2], &total_count[2]);
        getrawmonotonic(&times[0]);
    }

    for (i = TEST_SIZE; i >= 1; i--) {
        int target = rand(&seed) % i + 1;
        j = 0;

        list_for_each(pos, &head) {
            if (++j == target) break;
        }

        getrawmonotonic(&times[0]);
        list_del(pos);
        getrawmonotonic(&times[1]);
        calclock(times, &total_time[3], &total_count[3]);
    }

    printk("list_test      (random access)    : %5llu.%02llu ms (%llu times)\n", total_time[1] / MILLION, total_time[1] * 100 / MILLION % 100, total_count[1]);
    printk("list_test      (sequential access): %5llu.%02llu ms (%llu times)\n", total_time[2] / MILLION, total_time[2] * 100 / MILLION % 100, total_count[2]);
    printk("list_test      (insert)           : %5llu.%02llu ms (%llu times)\n", total_time[0] / MILLION, total_time[0] * 100 / MILLION % 100, total_count[0]);
    printk("list_test      (delete)           : %5llu.%02llu ms (%llu times)\n", total_time[3] / MILLION, total_time[3] * 100 / MILLION % 100, total_count[3]);
    printk("list_test      (total)            : \e[0;36m%5llu.%02llu ms\e[0m\n", (total_time[0] + total_time[1] + total_time[2] + total_time[3]) / MILLION, (total_time[0] + total_time[1] + total_time[2] + total_time[3]) * 100 / MILLION % 100);
}

void sqrt_list_test(void)
{
    static struct sqrt_list_node data[TEST_SIZE];
    int i;
    unsigned int seed = 42;
    unsigned long long total_time[10] = {0};
    unsigned long long total_count[10] = {0};
    struct sqrt_list_head *pos;
    struct sqrt_list_head head;
    struct timespec times[2];

    INIT_SQRT_LIST_HEAD(&head);

    for (i = 0; i < TEST_SIZE; i++) {
        struct sqrt_list_node *new = &data[i];
        int target = rand(&seed) % (i + 1);
        pos = &head;

        if (target > 0) {
            pos = sqrt_list_nth(&head, target);
        }

        getrawmonotonic(&times[0]);
        sqrt_list_add(&new->entry, pos);
        getrawmonotonic(&times[1]);
        calclock(times, &total_time[0], &total_count[0]);
    }

    i = 0;

    sqrt_list_for_each(pos, &head) {
        sqrt_list_entry(pos, struct sqrt_list_node, entry)->data = ++i;
    }

    for (i = 0; i < TEST_SIZE; i++) {
        int target = rand(&seed) % TEST_SIZE + 1;
        getrawmonotonic(&times[0]);
        pos = sqrt_list_nth(&head, target);
        WARN_ON(sqrt_list_entry(pos, struct sqrt_list_node, entry)->data != target);
        getrawmonotonic(&times[1]);
        calclock(times, &total_time[1], &total_count[1]);
    }

    i = 0;
    getrawmonotonic(&times[0]);

    sqrt_list_for_each(pos, &head) {
        WARN_ON(sqrt_list_entry(pos, struct sqrt_list_node, entry)->data != ++i);
        getrawmonotonic(&times[1]);
        calclock(times, &total_time[2], &total_count[2]);
        getrawmonotonic(&times[0]);
    }

    for (i = TEST_SIZE; i >= 1; i--) {
        int target = rand(&seed) % i + 1;
        pos = sqrt_list_nth(&head, target);
        getrawmonotonic(&times[0]);
        sqrt_list_del(pos);
        getrawmonotonic(&times[1]);
        calclock(times, &total_time[3], &total_count[3]);
    }

    printk("sqrt_list_test (random access)    : %5llu.%02llu ms (%llu times)\n", total_time[1] / MILLION, total_time[1] * 100 / MILLION % 100, total_count[1]);
    printk("sqrt_list_test (sequential access): %5llu.%02llu ms (%llu times)\n", total_time[2] / MILLION, total_time[2] * 100 / MILLION % 100, total_count[2]);
    printk("sqrt_list_test (insert)           : %5llu.%02llu ms (%llu times)\n", total_time[0] / MILLION, total_time[0] * 100 / MILLION % 100, total_count[0]);
    printk("sqrt_list_test (delete)           : %5llu.%02llu ms (%llu times)\n", total_time[3] / MILLION, total_time[3] * 100 / MILLION % 100, total_count[3]);
    printk("sqrt_list_test (total)            : \e[0;36m%5llu.%02llu ms\e[0m\n", (total_time[0] + total_time[1] + total_time[2] + total_time[3]) / MILLION, (total_time[0] + total_time[1] + total_time[2] + total_time[3]) * 100 / MILLION % 100);
}
