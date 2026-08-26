// Copyright 2021 Jeisson Hidalgo-Cespedes <jeisson.hidalgo@ucr.ac.cr> CC-BY-4

#ifndef QUEUE_H
#define QUEUE_H

#include <inttypes.h>
#include <pthread.h>
#include <stdbool.h>

/**
 * @brief A node for a queue. It contains a string and a pointer to the next
 * node.
*/
typedef struct queue_node {
  char* str;
  struct queue_node* next;
} queue_node_t;

/**
 * @brief A thread safe queue
 * @details Simple queue. It has a pointer to its head and a pointer to its
 * tail. It also has a count. To make it thread safe, the queue manages a mutex.
*/
typedef struct {
  pthread_mutex_t can_access_queue;
  queue_node_t* head;
  queue_node_t* tail;
  uint64_t count;
} queue_t;

/**
 * @brief Initialize a queue
 * @details Routine that initializes the queue's attributes. 
 * @param queue The queue
 * @returns A status code only defined by the initialization of the queue's
 * mutex. 0 means success
 */
int queue_init(queue_t* queue);

/**
 * @brief Destroy a queue
 * @details Remove all nodes of the queue and then destroy its mutex.
 * @param queue The queue
 * @returns A status code only defined by the destruction of the queue's mutex.
 * 0 means success
*/
int queue_destroy(queue_t* queue);

/**
 * @brief Check if the queue is empty
 * @details Check if the queue's head is NULL. This function is thread-safe.
 * @param queue The queue
 * @returns A boolean. true means the queue is empty
*/
bool queue_is_empty(queue_t* queue);

/**
 * @brief Enqueue 'str' in 'queue'
 * @details Create a new node for the queue. Copy 'str' in the new node and
 * then enqueue the node. This procedure is thread-safe.
 * @param queue The queue
 * @param str The string that will be enqueued
 * @returns A status code. 0 means success
*/
int queue_enqueue(queue_t* queue, const char* str);

/**
 * @brief Dequeue a node from 'queue' and copy its contents in 'str'
 * @details Copy the string of the queue's head in 'str', then remove the node
 * at the queue's head. This procedure is thread-safe.
 * @param queue The queue
 * @param str The string in which the queue's head will be returned.
 * @returns A status code. 0 means success
*/
int queue_dequeue(queue_t* queue, char* str);

/**
 * @brief Dequeue a node from 'queue' and don't return the node's string.
 * @details Remove the node currently pointed to by the queue's head. This
 * procedure is thread-safe.
 * @param queue The queue
 * @returns A status code. 0 means success
*/
int queue_dequeue_no_return(queue_t* queue);

/**
 * @brief Empty the queue
 * @details Remove all nodes from queue. This procedure is thread-safe.
 * @param queue The queue
*/
void queue_clear(queue_t* queue);

#endif  // QUEUE_H
