#include "array_size.h"
#include <stdlib.h>

/**
 * @brief Increases the array capacity.
 * @details This routine calls realloc soliciting ten times the old capacity.
 * @param array The array of sizes.
 * @returns A status variable. 0 means success.
*/
int array_size_increase_capacity(array_size_t* array);

array_size_t* array_size_create(int capacity) {
  array_size_t *array = (array_size_t*) malloc(1 * sizeof(array_size_t));
  if (array) {
    array->count = 0;
    array->capacity = capacity;
    array->elements = (size_t*) calloc(capacity, sizeof(size_t));
    if (array->elements) {
      return array;
    } else {
      free(array);
      return NULL;
    }
  }
}

void array_size_destroy(array_size_t* array) {
  free(array->elements);
  free(array);
}

int array_size_append(array_size_t* array, size_t element) {
  if (array->count == array->capacity) {
    if (array_size_increase_capacity(array) != EXIT_SUCCESS) {
      return EXIT_FAILURE;
    }
  }
  array->elements[array->count++] = element;
  return EXIT_SUCCESS;
}

int array_size_increase_capacity(array_size_t* array) {
  size_t new_capacity = 10* array->capacity;
  size_t* new_elements = (size_t*) realloc(array->elements,
    new_capacity * sizeof(size_t));
  if (new_elements) {
    array->capacity = new_capacity;
    array->elements = new_elements;
    return EXIT_SUCCESS;
  } else {
    return EXIT_FAILURE;
  }
}