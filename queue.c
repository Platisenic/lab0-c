#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */

#define list_for_each_entry_safe_reverse(entry, safe, head, member)        \
    for (entry = list_entry((head)->prev, __typeof__(*entry), member),     \
        safe = list_entry(entry->member.prev, __typeof__(*entry), member); \
         &entry->member != (head); entry = safe,                           \
        safe = list_entry(safe->member.prev, __typeof__(*entry), member))


#define list_for_range_entry_safe(entry, safe, start, end, member)         \
    for (entry = list_entry(start, __typeof__(*entry), member),            \
        safe = list_entry(entry->member.next, __typeof__(*entry), member); \
         &entry->member != (end); entry = safe,                            \
        safe = list_entry(safe->member.next, __typeof__(*entry), member))

/* Create an empty queue */
struct list_head *q_new()
{
    struct list_head *h = malloc(sizeof(struct list_head));
    if (!h)
        return NULL;

    INIT_LIST_HEAD(h);
    return h;
}

/* Free all storage used by queue */
void q_free(struct list_head *l)
{
    if (!l)
        return;

    element_t *li, *next;

    list_for_each_entry_safe (li, next, l, list)
        q_release_element(li);

    free(l);
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head)
        return false;

    element_t *ele = malloc(sizeof(element_t));
    if (!ele)
        return false;

    ele->value = strdup(s);
    if (!ele->value) {
        q_release_element(ele);
        return false;
    }
    list_add(&ele->list, head);
    return true;
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head)
        return false;

    element_t *ele = malloc(sizeof(element_t));
    if (!ele)
        return false;

    ele->value = strdup(s);
    if (!ele->value) {
        q_release_element(ele);
        return false;
    }
    list_add_tail(&ele->list, head);
    return true;
}

/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;

    element_t *ele = list_first_entry(head, element_t, list);
    if (sp) {
        strncpy(sp, ele->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }
    list_del_init(&ele->list);
    return ele;
}

/* Remove an element from tail of queue */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;

    element_t *ele = list_last_entry(head, element_t, list);
    if (sp) {
        strncpy(sp, ele->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }
    list_del_init(&ele->list);
    return ele;
}

/* Return number of elements in queue */
int q_size(struct list_head *head)
{
    if (!head)
        return 0;

    int len = 0;
    struct list_head *li;

    list_for_each (li, head)
        len++;
    return len;
}

/* Delete the middle node in queue */
// https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/
bool q_delete_mid(struct list_head *head)
{
    if (!head || list_empty(head))
        return false;

    struct list_head *slow = head;
    struct list_head *fast = head;
    element_t *ele;
    bool first = true;

    while (first || (fast != head && fast->next != head)) {
        first = false;
        slow = slow->next;
        fast = fast->next->next;
    }
    ele = list_entry(slow, element_t, list);
    list_del(slow);
    q_release_element(ele);
    return true;
}

/* Delete all nodes that have duplicate string */
// https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/
bool q_delete_dup(struct list_head *head)
{
    if (!head || list_empty(head))
        return false;

    element_t *entry, *safe;
    struct list_head *node1, *node2;

    q_sort(head, false);

    for (node1 = head->next; node1 != head; node1 = node2) {
        node2 = node1->next;
        while (node2 != head &&
               !strcmp(list_entry(node1, element_t, list)->value,
                       list_entry(node2, element_t, list)->value)) {
            node2 = node2->next;
        }
        if (node2 != node1->next) {
            list_for_range_entry_safe(entry, safe, node1, node2, list)
            {
                list_del(&entry->list);
                q_release_element(entry);
            }
        }
    }

    return true;
}

/* Swap every two adjacent nodes */
// https://leetcode.com/problems/swap-nodes-in-pairs/
void q_swap(struct list_head *head)
{
    if (!head || list_empty(head))
        return;

    struct list_head *n1 = head->next;
    struct list_head *n0, *n2, *n3;
    // n0 n1 n2 n3
    //     ^
    // n0 n2 n1 n3
    while (n1 != head && n1->next != head) {
        n0 = n1->prev;
        n2 = n1->next;
        n3 = n2->next;

        n0->next = n2;
        n2->next = n1;
        n2->prev = n0;
        n1->next = n3;
        n1->prev = n2;
        n3->prev = n1;

        n1 = n3;
    }
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head)
{
    if (!head || list_empty(head))
        return;

    struct list_head *node, *safe;

    list_for_each_safe (node, safe, head) {
        list_move(node, head);
    }
}

static struct list_head *reverse(struct list_head *head)
{
    struct list_head *cur = head;
    struct list_head *prev = NULL, *next;
    while (cur) {
        next = cur->next;
        cur->next = prev;
        cur->prev = next;
        prev = cur;
        cur = next;
    }
    return prev;
}

static struct list_head *reverseK(struct list_head *head, int k)
{
    struct list_head *cur = head;
    struct list_head *prev = head, *rev_head, *node;
    int t = 0;

    while (cur && t < k) {
        prev = cur;
        cur = cur->next;
        t++;
    }
    if (t == k) {
        prev->next = NULL;
        rev_head = reverse(head);
        node = reverseK(cur, k);
        head->next = node;
        node->prev = head;
    } else {
        rev_head = head;
    }
    return rev_head;
}

/* Reverse the nodes of the list k at a time */
// https://leetcode.com/problems/reverse-nodes-in-k-group/
void q_reverseK(struct list_head *head, int k)
{
    if (!head || list_empty(head))
        return;

    struct list_head *first = head->next;
    struct list_head *last = head->prev;
    struct list_head *resFirst, *resLast, *tmp;

    first->prev = NULL;
    last->next = NULL;

    tmp = resLast = resFirst = reverseK(first, k);
    while (tmp) {
        resLast = tmp;
        tmp = tmp->next;
    }

    head->next = resFirst;
    head->prev = resLast;
    resFirst->prev = head;
    resLast->next = head;
}

static struct list_head *merge(struct list_head *h1,
                               struct list_head *h2,
                               bool descend)
{
    struct list_head *first;
    struct list_head dummy;
    struct list_head *cur = &dummy;
    while (h1 && h2) {
        if (!descend) {
            if (strcmp(list_entry(h1, element_t, list)->value,
                       list_entry(h2, element_t, list)->value) < 0) {
                cur->next = h1;
                h1->prev = cur;
                h1 = h1->next;
            } else {
                cur->next = h2;
                h2->prev = cur;
                h2 = h2->next;
            }
        } else {
            if (strcmp(list_entry(h1, element_t, list)->value,
                       list_entry(h2, element_t, list)->value) > 0) {
                cur->next = h1;
                h1->prev = cur;
                h1 = h1->next;
            } else {
                cur->next = h2;
                h2->prev = cur;
                h2 = h2->next;
            }
        }
        cur = cur->next;
    }
    if (h1) {
        cur->next = h1;
        h1->prev = cur;
    }
    if (h2) {
        cur->next = h2;
        h2->prev = cur;
    }
    first = dummy.next;
    first->prev = NULL;
    return first;
}

static struct list_head *mergeSort(struct list_head *head, bool descend)
{
    if (!head->next)
        return head;

    struct list_head *midprev = head, *mid = head, *fast = head;
    struct list_head *left, *right, *resHead;

    while (fast && fast->next) {
        midprev = mid;
        mid = mid->next;
        fast = fast->next->next;
    }
    midprev->next = NULL;
    mid->prev = NULL;

    left = mergeSort(head, descend);
    right = mergeSort(mid, descend);
    resHead = merge(left, right, descend);
    return resHead;
}

/* Sort elements of queue in ascending/descending order */
void q_sort(struct list_head *head, bool descend)
{
    if (!head || list_empty(head))
        return;

    struct list_head *first = head->next;
    struct list_head *last = head->prev;
    struct list_head *resFirst, *resLast, *tmp;

    first->prev = NULL;
    last->next = NULL;

    tmp = resLast = resFirst = mergeSort(first, descend);
    while (tmp) {
        resLast = tmp;
        tmp = tmp->next;
    }

    head->next = resFirst;
    head->prev = resLast;
    resFirst->prev = head;
    resLast->next = head;
}

/* Remove every node which has a node with a strictly less value anywhere to
 * the right side of it */
// https://leetcode.com/problems/remove-nodes-from-linked-list/
int q_ascend(struct list_head *head)
{
    if (!head || list_empty(head))
        return 0;

    element_t *entry, *safe;
    element_t *min_ele = list_last_entry(head, element_t, list);

    list_for_each_entry_safe_reverse(entry, safe, head, list)
    {
        if (strcmp(entry->value, min_ele->value) <= 0) {
            min_ele = entry;
        } else {
            list_del(&entry->list);
            q_release_element(entry);
        }
    }
    return q_size(head);
}

/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
// https://leetcode.com/problems/remove-nodes-from-linked-list/
int q_descend(struct list_head *head)
{
    if (!head || list_empty(head))
        return 0;

    element_t *entry, *safe;
    element_t *max_ele = list_last_entry(head, element_t, list);

    list_for_each_entry_safe_reverse(entry, safe, head, list)
    {
        if (strcmp(entry->value, max_ele->value) >= 0) {
            max_ele = entry;
        } else {
            list_del(&entry->list);
            q_release_element(entry);
        }
    }
    return q_size(head);
}

/* Merge all the queues into one sorted queue, which is in ascending/descending
 * order */
// https://leetcode.com/problems/merge-k-sorted-lists/
int q_merge(struct list_head *head, bool descend)
{
    if (!head || list_empty(head))
        return 0;

    struct list_head *q1 = list_entry(head->next, queue_contex_t, chain)->q;
    int len = q_size(q1);

    if (list_is_singular(head))
        return q_size(q1);

    for (struct list_head *qchain = head->next->next; qchain != head;
         qchain = qchain->next) {
        struct list_head *q2 = list_entry(qchain, queue_contex_t, chain)->q;
        len += q_size(q2);
        list_splice_tail_init(q2, q1);
    }

    q_sort(q1, descend);

    return len;
}
