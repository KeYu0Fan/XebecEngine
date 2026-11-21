#include "sort.h"
#if COM_USE(CONFIG_ALGO_SORT)

#include <string.h>

static void byte_swap(void* a, void* b, qty_t size)
{
    unsigned char* pa = (unsigned char*)a;
    unsigned char* pb = (unsigned char*)b;
    qty_t i;
    for (i = 0; i < size; i++) {
        unsigned char t = pa[i];
        pa[i] = pb[i];
        pb[i] = t;
    }
}

static void byte_copy(void* dst, const void* src, qty_t size)
{
    memcpy(dst, src, size);
}

/* Lomuto 分区, 枢轴取最后一个元素
 * 语义: cmp(a,b) > 0 表示 a > b, <= 0 表示 a <= b
 */
static void quick_recursive(unsigned char* src, qty_t count, qty_t elem, cmp_t(*cmp)(void*, void*))
{
    qty_t i, j;
    void* pivot;

    if (count < 2) {
        return;
    }

    pivot = src + (count - 1) * elem;
    i = 0;

    for (j = 0; j < count - 1; j++) {
        void* cur = src + j * elem;
        /* 当前元素 <= 枢轴 (cmp <= 0), 交换到左侧 */
        if (cmp(cur, pivot) <= 0) {
            if (i != j) {
                byte_swap(src + i * elem, cur, elem);
            }
            i++;
        }
    }
    byte_swap(src + i * elem, pivot, elem);

    quick_recursive(src, i, elem, cmp);
    quick_recursive(src + (i + 1) * elem, count - i - 1, elem, cmp);
}

void sort_quick(void* src, qty_t count, qty_t elem, cmp_t(*cmp)(void*, void*))
{
    if (src == NULL || count < 2 || elem == 0 || cmp == NULL) {
        return;
    }
    quick_recursive((unsigned char*)src, count, elem, cmp);
}

void sort_insert(void* src, qty_t count, qty_t elem, cmp_t(*cmp)(void*, void*))
{
    qty_t i, j;
    unsigned char* p = (unsigned char*)src;

    if (p == NULL || count < 2 || elem == 0 || cmp == NULL) {
        return;
    }

    for (i = 1; i < count; i++) {
        j = i;
        /* 前元素 > 后元素时交换: cmp > 0 */
        while (j > 0 && cmp(p + (j - 1) * elem, p + j * elem) > 0) {
            byte_swap(p + (j - 1) * elem, p + j * elem, elem);
            j--;
        }
    }
}

static void merge_recursive(unsigned char* src, unsigned char* temp, qty_t count, qty_t elem, cmp_t(*cmp)(void*, void*))
{
    qty_t mid, i, j, k;
    unsigned char* left;
    unsigned char* right;

    if (count < 2) {
        return;
    }

    mid = count / 2;
    left = src;
    right = src + mid * elem;

    merge_recursive(left, temp, mid, elem, cmp);
    merge_recursive(right, temp, count - mid, elem, cmp);

    i = 0;
    j = 0;
    k = 0;
    while (i < mid && j < count - mid) {
        void* l = left + i * elem;
        void* r = right + j * elem;
        /* 稳定性: 相等时优先取左半段 (cmp <= 0 即 l <= r) */
        if (cmp(l, r) <= 0) {
            byte_copy(temp + k * elem, l, elem);
            i++;
        }
        else {
            byte_copy(temp + k * elem, r, elem);
            j++;
        }
        k++;
    }
    while (i < mid) {
        byte_copy(temp + k * elem, left + i * elem, elem);
        i++;
        k++;
    }
    while (j < count - mid) {
        byte_copy(temp + k * elem, right + j * elem, elem);
        j++;
        k++;
    }

    byte_copy(src, temp, count * elem);
}

bool_t sort_merge(void* src, qty_t count, qty_t elem, cmp_t(*cmp)(void*, void*), void* temp)
{
    if (count < 2) {
        return 1;
    }
    if (src == NULL || elem == 0 || cmp == NULL || temp == NULL) {
        return 0;
    }
    merge_recursive((unsigned char*)src, (unsigned char*)temp, count, elem, cmp);
    return 1;
}

#endif