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
	// TODO: check if malloc succeeded

	node->prev = NULL;
	node->next = NULL;
	node->status = *status;

	if (head != NULL) {
		(*head)->prev = node;
		node->next = *head;
	}

	*head = node;

	return *head;
}

bool list_add(er_node_t *head, er_node_t *item) {
	er_node_t *current = head;

	if (current != NULL) {
		while (current->next != NULL) {
			current = current->next;
		}

		current = current->next;  // Step into NULL
	}

	current = malloc(sizeof(er_node_t));
	if (current == NULL) {
		return false;
	}

	current = item;

	return true;
}

bool list_remove(er_node_t *head) {
	if (head == NULL) {
		return false;
	}

	if ((head)->prev != NULL) {
		(head)->prev->next = (head)->next;
	}
	if ((head)->next != NULL) {
		(head)->next->prev = (head)->prev;
	}

	free(head);
	return true;
}