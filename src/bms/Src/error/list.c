/**
 * @file		list.c
 * @brief		This file contains the functions to manage the error linked list
 *
 * @date		Dec 7, 2019
 * @author	Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#include "error/list.h"

#include <stdlib.h>

void list_init(er_node_t *head) {
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
er_node_t *list_insert(er_node_t **head, error_status_t *status) {
	er_node_t *node = (er_node_t *)malloc(sizeof(er_node_t));

	if (node == NULL) {
		return NULL;
	}

	node->status = *status;
	node->prev = NULL;
	node->next = *head;

	if (*head != NULL) {
		(*head)->prev = node;
	}

	*head = node;

	return *head;
}

bool list_remove(er_node_t **head, er_node_t *node) {
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

	free(node);
	return true;
}