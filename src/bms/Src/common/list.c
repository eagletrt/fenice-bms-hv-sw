/**
 * @file		list.c
 * @brief		This file contains a generic implementation of a double linked list
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
 * @brief		Inserts a node at the beginning of the list
 * 
 * @param head			List head
 * @param data			Pointer to data element to be inserted
 * @param data_size	Size of data field
 */
node_t *list_insert(node_t **head, void *data, size_t data_size) {
	node_t *node = (node_t *)malloc(sizeof(node_t));

	node->data = malloc(data_size);

	if (node == NULL || node->data == NULL) {
		return NULL;
	}

	// Copy each byte to node->data
	for (uint16_t i = 0; i < data_size; i++) {
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

/**
 * @brief Removes a given node
 * 
 * @param	head	list head
 * @param node	the node to delete
 */
// TODO: is head really necessary here?
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

uint8_t list_count(node_t *head) {
	node_t *node = head;
	uint8_t count = 0;

	while (node != NULL) {
		count++;
		node = node->next;
	}

	return count;
}

void *list_get_nth(node_t *head, uint8_t index) {
	node_t *node = head;

	// TODO: Check logic
	uint8_t i = 0;
	while (node != NULL) {
		if (i == index) {
			return node->data;
		}
		i++;
		node = node->next;
	}

	return NULL;
}