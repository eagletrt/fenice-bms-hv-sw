/**
 * @file		list.c
 * @brief		This file contains the functions to manage the error linked list
 *
 * @date		Dec 7, 2019
 * @author	Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#include "common/list.h"

#include <stdlib.h>

void list_init(node_t *head) {
	head = NULL;
}

/**
 * @brief		Inserts at the beginning of the list
 * @details	The function itselfs creates the values for 
 * 
 * 					------------
 *					|   prev   |
 *					|   HEAD   |
 *					|   next   |
 *					------------
 *					     ^
 *					     |
 *					     V
 *					------------
 *					|   prev   |
 *					|   node   |
 *					|   next   |
 *					------------
 * 
 * @param head		List head
 * @param status	Error status reference passed by address
 */
node_t *list_insert(node_t **head, void *data, size_t data_size) {
	node_t *node = (node_t *)malloc(sizeof(node_t));

	node->data = malloc(sizeof(data));

	if (node == NULL) {
		return NULL;
	}

	// Copy each byte to node->data
	for (uint8_t i = 0; i < data_size; i++) {
		*(char *)(node->data + i) = *(char *)(data + i);
	}

	node->prev = NULL;
	node->next = *head;

	if (*head != NULL) {
		(*head)->prev = node;
	}

	*head = node;

	return *head;
}

bool list_remove(node_t **head, node_t *node) {
	if (*head == NULL || node == NULL) {
		return false;
	}

	if (*head == node) {
		*head = node->next;
	}

	if (node->prev != NULL) {
		node->prev->next = node->next;
	}
	if (node->next != NULL) {
		node->next->prev = node->prev;
	}

	free(node->data);
	free(node);
	return true;
}