#include <stdlib.h>
#include <stdio.h>

#include "linked_list.h"

linked_list *create_list(int x, int y)
{
	linked_list *head = malloc(sizeof(linked_list));
	head->x = x;
	head->y = y;
	head->next = NULL;

	return head;
}

int get_len_list(linked_list * head)
{
	int len;
	linked_list *curr = head;
	for (len = 1; curr->next != NULL; len++) {
		curr = curr->next;
	}
	return len;
}

void add_to_list(linked_list * head, int x, int y)
{
	linked_list *curr = head;
	while (curr->next != NULL) {
		curr = curr->next;
	}
	curr->next = malloc(sizeof(linked_list));
	curr->next->x = x;
	curr->next->y = y;
	curr->next->next = NULL;
}

int is_in_list(linked_list * head, int x, int y)
{
	linked_list *curr = head;
	while (curr != NULL) {
		if (curr->x == x && curr->y == y) {
			return 1;
		}
		curr = curr->next;
	}
	return 0;
}

int *get_top_left_list(linked_list * head)
{
	int x = -1;
	int y = -1;

	linked_list *curr = head;
	while (curr != NULL) {
		if (x < 0 && y < 0) {
			x = curr->x;
			y = curr->y;
		} else if (x > curr->x) {
			x = curr->x;
			y = curr->y;
		} else if (x == curr->x) {
			if (y > curr->y) {
				y = curr->y;
			}
		}
		curr = curr->next;
	}
	int *result = malloc(sizeof(int) * 2);
	result[0] = x;
	result[1] = y;

	return result;
}

void print_list(linked_list * head)
{
	linked_list *curr = head;
	while (curr != NULL) {
		printf("x: %d, y: %d\n", curr->x, curr->y);
		curr = curr->next;
	}
}

void free_list(linked_list * head)
{
	linked_list *tmp;
	while (head != NULL) {
		tmp = head;
		head = head->next;
		free(tmp);
	}
}
