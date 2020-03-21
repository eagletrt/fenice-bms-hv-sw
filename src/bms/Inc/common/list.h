/**
 * @file		list.h
 * @brief		This file contains a generic implementation of a double linked list
 *
 * @date		Dec 7, 2019
 * @author	Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#ifndef LIST_H
#define LIST_H

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct node {
	struct node *prev;
	void *data;
	struct node *next;
} node_t;

void list_init(node_t *head);

node_t *list_insert(node_t **head, void *data, size_t data_size);
bool list_remove(node_t **head, node_t *node);
uint8_t list_count(node_t *head);
node_t *list_get_nth(node_t *head, uint8_t index);

#endif