/**
 * @file		list.h
 * @brief		This file contains the functions to manage the error linked list
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
#endif