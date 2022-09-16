#ifndef LINKED_LIST_H
#define LINKED_LIST_H

typedef struct linked_list {
    int x;
    int y;
    struct linked_list *next;
} linked_list;

extern linked_list *create_list(int x, int y);
extern int get_len_list(linked_list * head);
extern void add_to_list(linked_list * head, int x, int y);
extern int is_in_list(linked_list * head, int x, int y);
extern int *get_top_left_list(linked_list * head);
extern void print_list(linked_list * head);
extern void free_list(linked_list * head);

#endif				/* LINKED_LIST_H */
