/**
 * @file		list.c
 * @brief		This file contains the functions to manage the error linked list
 *
 * @date		Dec 7, 2019
 * @author	Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#include "error/list.h"
#include <stdlib.h>

void er_node_init(er_node_t *node, void *ref, uint8_t ref_index, error_t type) {
	node->ref = ref;
	node->ref_index = ref_index;
	node->next = NULL;
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

bool list_pop(er_node_t **head) {
	er_node_t *next_node = NULL;

	if (*head == NULL) {
		return false;
	}

	next_node = (*head)->next;
	free(*head);
	*head = next_node;

	return true;
}

bool list_remove(er_node_t **head, er_node_t *node) {
	er_node_t *current = *head;
	er_node_t *temp_node = NULL;

	if (*head == node) {
		return list_pop(head);
	}

	while (current->next != node) {
		current = current->next;
	}

	temp_node = current->next;
	current->next = temp_node->next;
	free(temp_node);

	return true;
}

er_node_t *list_find(er_node_t *head, error_t type, void *ref,
					 uint8_t ref_index) {
	er_node_t *current = head;

	while (current != NULL) {
		if (current->ref == ref && current->ref_index &&
			current->status.type == type) {
			return current;
		}

		current = current->next;
	}
	return NULL;
}