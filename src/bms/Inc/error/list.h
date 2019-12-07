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
#include "error/error_def.h"

typedef struct er_node {
	void *ref;
	uint8_t ref_index;
	error_status_t status;
	struct er_node *next;
} er_node_t;

void er_node_init(er_node_t *node, void *ref, uint8_t ref_index, error_t type);
bool list_add(er_node_t *head, er_node_t *item);
bool list_pop(er_node_t **head);
bool list_remove(er_node_t **head, er_node_t *node);
er_node_t *list_find(er_node_t *head, error_t type, void *ref,
					 uint8_t ref_index);

#endif