// Copyright 2021 Jeisson Hidalgo-Cespedes <jeisson.hidalgo@ucr.ac.cr> CC-BY-4

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <semaphore.h>

#include "queue.h"

/**
 * @brief Remove the queue's head without thread safety.
 * @details Thread-unsafe procedure used in some thread-safe procedures to
 * prevent "sleeping beauties" (threads getting stuck in a mutex forever).
 * @param queue The queue
 * @see queue_dequeue
 * @see queue_clear
 * @see queue_is_empty_unsafe
*/
void queue_remove_first_unsafe(queue_t* queue);

/**
 * @brief Check if the queue is empty without thread safety.
 * @details Same as queue_remove_first_unsafe
 * @param queue The queue
 * @returns Same as queue_is_empty
 * @see queue_is_empty
 * @see queue_dequeue
 * @see queue_clear
 * @see queue_remove_first_unsafe
*/
bool queue_is_empty_unsafe(queue_t* queue);

int queue_init(queue_t* queue) {
  assert(queue);
  int error = pthread_mutex_init(&queue->can_access_queue, NULL);
  queue->head = NULL;
  queue->tail = NULL;
  queue->count = 0;
  return error;
}

int queue_destroy(queue_t* queue) {
  queue_clear(queue);
  int error = pthread_mutex_destroy(&queue->can_access_queue);
  return error;
}

bool queue_is_empty(queue_t* queue) {
  assert(queue);
  pthread_mutex_lock(&queue->can_access_queue);
  bool result = queue->head == NULL;
  pthread_mutex_unlock(&queue->can_access_queue);
  return result;
}

bool queue_is_empty_unsafe(queue_t* queue) {
  assert(queue);
  return queue->head == NULL;
}

int queue_enqueue(queue_t* queue, const password_block_t block) {
  assert(queue);
  int error = EXIT_SUCCESS;

  queue_node_t* new_node = (queue_node_t*) calloc(1, sizeof(queue_node_t));
  if (new_node) {
    new_node->block.start = block.start;
    new_node->block.finish = block.finish;
    new_node->block.password_length = block.password_length;
    pthread_mutex_lock(&queue->can_access_queue);
    if (queue->tail) {
      queue->tail = queue->tail->next = new_node;
    } else {
      queue->head = queue->tail = new_node;
    }
    ++queue->count;
    pthread_mutex_unlock(&queue->can_access_queue);
  } else {
    error = EXIT_FAILURE;
  }

  return error;
}

int queue_dequeue(queue_t* queue, password_block_t* block) {
  assert(queue);
  int error = 0;

  pthread_mutex_lock(&queue->can_access_queue);
  if (!queue_is_empty_unsafe(queue)) {
    if (block) {
      block->start = queue->head->block.start;
      block->finish = queue->head->block.finish;
      block->password_length = queue->head->block.password_length;
      queue_remove_first_unsafe(queue);
    }
  } else {
    error = EXIT_FAILURE;
  }
  pthread_mutex_unlock(&queue->can_access_queue);

  return error;
}

int queue_dequeue_no_return(queue_t* queue) {
  assert(queue);
  int error = 0;

  pthread_mutex_lock(&queue->can_access_queue);
  if (!queue_is_empty_unsafe(queue)) {
    queue_remove_first_unsafe(queue);
  } else {
    error = EXIT_FAILURE;
  }
  pthread_mutex_unlock(&queue->can_access_queue);

  return error;
}

void queue_remove_first_unsafe(queue_t* queue) {
  assert(queue);
  assert(!queue_is_empty_unsafe(queue));
  queue_node_t* node = queue->head;
  queue->head = queue->head->next;
  --queue->count;
  free(node);
  if (queue->head == NULL) {
    queue->tail = NULL;
  }
}

void queue_clear(queue_t* queue) {
  assert(queue);
  pthread_mutex_lock(&queue->can_access_queue);
  while (!queue_is_empty_unsafe(queue)) {
    queue_remove_first_unsafe(queue);
  }
  pthread_mutex_unlock(&queue->can_access_queue);
}
