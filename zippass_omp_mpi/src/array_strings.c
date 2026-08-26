// Copyright <2023> <Andrés Serrano>

#include "array_strings.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Private routine
/**
 * @brief Increases the array capacity.
 * @details This routine calls realloc soliciting ten times the old capacity.
 * @param array The array of strings.
 * @returns A status variable. 0 means success.
*/
int array_strings_increase_capacity(array_strings_t* array);
#if 0
typedef struct array_of_strings {
  size_t capacity;
  size_t count;
  char** strings;
} array_strings_t;
#endif

array_strings_t* array_strings_create(int capacity) {
  // Allocate array_strings_t
  array_strings_t* array = (array_strings_t*)
    malloc(1 * sizeof(array_strings_t));
  if (array) {
    array->count = 0;
    // Initialize array with a capacity
    array->capacity = capacity;
    // Allocate array of strings
    array->strings = (char**) calloc(capacity, sizeof(char**));
    if (array->strings) {
      // Allocate 'capacity' strings
      for (size_t i = 0; i < array->capacity; ++i) {
        array->strings[i] = (char*) calloc(100, sizeof(char));
        if (!array->strings[i]) {
          free(array->strings);
          free(array);
          fprintf(stderr,
          "Error: Couldn't allocate a string while creating the array\n");
          return NULL;
        }
      }
      return array;
    } else {
      free(array);
      fprintf(stderr, "Error: Couldn't allocate array of strings");
      return NULL;
    }
  } else {
    fprintf(stderr, "Error: Couldn't allocate dynamic array of strings\n");
    return NULL;
  }
}

void array_strings_destroy(array_strings_t* array) {
  assert(array);

  for (size_t i = 0; i < array->capacity; ++i) {
    free(array->strings[i]);
  }
  free(array->strings);

  array->capacity = 0;
  array->count = 0;
  free(array);
  return;
}

int array_strings_append(array_strings_t* array, char* str) {
  assert(array);
  if (array->count == array->capacity) {
    if (array_strings_increase_capacity(array) != EXIT_SUCCESS) {
      return EXIT_FAILURE;
    }
  }
  snprintf(array->strings[array->count++], strlen(str)+1, "%s", str);
  return EXIT_SUCCESS;
}

int array_strings_set(array_strings_t* array, char* str, size_t index) {
  assert(array);
  assert(index <= array->capacity);
  if (array->count == array->capacity) {
    if (array_strings_increase_capacity(array) != EXIT_SUCCESS) {
      return EXIT_FAILURE;
    }
  }

  snprintf(array->strings[index], strlen(str)+1, "%s", str);
  ++array->count;
  return EXIT_SUCCESS;
}

#if 0
size_t array_strings_get_count(array_strings_t* array) {
  return array->count;
}

size_t array_strings_get_capacity(array_strings_t* array) {
  return array->capacity;
}

void array_strings_get_string(array_strings_t* array, size_t i, char* ret) {
  snprintf(ret, strlen(array->strings[i])+1, "%s", array->strings[i]);
}
#endif

// private routines

int array_strings_increase_capacity(array_strings_t* array) {
  size_t new_capacity = 10 * array->capacity;
  char** new_strings = (char**)
    realloc(array->strings, new_capacity * sizeof(char*));
  if (new_strings) {
    array->capacity = new_capacity;
    array->strings = new_strings;
    for (size_t i = array->count; i < array->capacity; ++i) {
      array->strings[i] = (char*) calloc(100, sizeof(char));
      if (!array->strings[i]) {
        return EXIT_FAILURE;
      }
    }
    return EXIT_SUCCESS;
  } else {
    fprintf(stderr, "Error: Couldn't increase array's capacity\n");
    return EXIT_FAILURE;
  }
}

int array_strings_resize(array_strings_t* array, size_t new_size) {
  char **new_strings = (char**) realloc(array->strings,
    new_size * sizeof(char*));
  if (new_strings) {
    array->capacity = new_size;
    array->strings = new_strings;
    for (size_t i = array->count; i < array->capacity; ++i) {
      array->strings[i] = (char*) calloc(100, sizeof(char));
      if (!array->strings[i]) {
        return EXIT_FAILURE;
      }
    }
    return EXIT_SUCCESS;
  }
}
