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
	struct er_node *prev;
	error_status_t status;
	struct er_node *next;
} er_node_t;

void list_init(er_node_t *head);

er_node_t *list_insert(er_node_t **head, error_status_t *status);
bool list_remove(er_node_t **head, er_node_t *node);
#endif