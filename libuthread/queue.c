#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "queue.h"

typedef struct node {
	void *data;
	struct node *prev;
	struct node *next;
} node;

typedef struct node* node_t;

node_t node_create(void* data) {
	node_t new_node = (node_t)malloc(sizeof(node));
	if (!new_node) return NULL;
	new_node->data = data;
	new_node->prev = new_node->next = NULL;

	return new_node;
}

struct queue {
	int length;
	node_t head;
	node_t tail;
};

queue_t queue_create(void)
{
	queue_t new_queue = (queue_t)malloc(sizeof(struct queue));
	new_queue->length = 0;
	new_queue->head = new_queue->tail = NULL;

	if (!new_queue) return NULL;
	return new_queue;
}

int queue_destroy(queue_t queue)
{
	if (queue == NULL || queue->length != 0) return -1;

	free(queue);
	queue = NULL;
	return 0;
}

int queue_enqueue(queue_t queue, void *data)
{
	node_t temp = node_create(data);

	if (queue == NULL || data == NULL || !temp) return -1;

	if (queue->head == NULL && queue->tail == NULL) {
		queue->head = queue->tail = temp;
	} else {
		queue->tail->next = temp;
		temp->prev = queue->tail;
		queue->tail = temp;
	}
	
	queue->length++;
	return 0;
}

int queue_dequeue(queue_t queue, void **data)
{
	if (queue == NULL || data == NULL || queue->length == 0) return -1;

	node_t temp = queue->head;

	if (queue->head == queue->tail) {
		*data = queue->head->data;
		queue->head = queue->tail = NULL;
	} else {
		*data = queue->head->data;
		queue->head = queue->head->next;
		queue->head->prev = NULL;
	}

	queue->length--;
	free(temp);
	temp = NULL;
	return 0;
}

int queue_delete(queue_t queue, void *data)
{
	if (queue == NULL || data == NULL || queue->length == 0) return -1;

	node_t temp = queue->head;

	// check if @data is in the queue
	while (temp->data != data) {
		if(temp->next == NULL) {
			return -1;
    	} else {
			temp = temp->next;             
    	}
	}
	
	// if @data is in the queue
	if (queue->head == queue->tail || temp == queue->head) {
		queue_dequeue(queue, (void**)&data);
		return 0;
	} else if (temp == queue->tail) {
		queue->tail = queue->tail->prev;
		queue->tail->next = NULL;
	} else {
		temp->prev->next = temp->next;
		temp->next->prev = temp->prev;
	}

	queue->length--;
	free(temp);
	temp = NULL;
	return 0;
}

int queue_iterate(queue_t queue, queue_func_t func, void *arg, void **data)
{
	if (queue == NULL || func == NULL) return -1;

	node_t current = queue->head;
	node_t next;

	while (current != NULL) {
		next = current->next;

		if (func(queue, current->data, arg) == 1) {
			if (data != NULL) *data = current->data;
			break;
		}

		current = next;
	}

	return 0;
}

int queue_length(queue_t queue)
{
	if (queue == NULL) return -1;

	return queue->length;
}

