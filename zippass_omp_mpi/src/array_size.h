#ifndef ARRAY_SIZE_H
#define ARRAY_SIZE_H

#include <stddef.h>

/**
 * @brief A dynamic array of size_t
 * @details This struct contains an array of size_t and two variables
 * to keep count of how many numbers it currently has (count) and how many
 * numbers it can currently hold (capacity). Thanks to those two variables, this
 * struct will automatically increase the capacity of the array of size_t, if
 * neccesary.
*/
typedef struct {
  size_t capacity;
  size_t count;
  size_t* elements;
} array_size_t;



/**
 * @brief Constructor for array_size_t datatypes.
 * @details Allocates an array_size_t and initializes it with capacity 1.
 * @returns A pointer to an array_size_t datatype
*/
array_size_t* array_size_create(int capacity);

/**
 * @brief Destructor for array_size_t datatypes.
 * @details Frees all memory allocated by the array and then frees the
 * array_size_t itself.
 * @param array The array of sizes
*/
void array_size_destroy(array_size_t* array);

/**
 * @brief Add a size to the end of the array.
 * @details Appends a size to the end of the array and reallocates memory if
 * neccesary.
 * @param array The array of sizes
 * @param str The size that is going to be appended
 * @returns An error code. 0 means the size was successfully appended.
*/
int array_size_append(array_size_t* array, size_t element);

#endif