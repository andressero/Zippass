// Copyright <2023> <Andrés Serrano>

#ifndef ARRAY_STRINGS_H
#define ARRAY_STRINGS_H

#include <stddef.h>

typedef struct array_of_strings array_strings_t;

/**
 * @brief Constructor for array_strings_t datatypes.
 * @details Allocates an array_strings_t and initializes it with capacity 1.
 * @returns A pointer to an array_strings_t datatype
*/
array_strings_t* array_strings_create();

/**
 * @brief Destructor for array_strings_t datatypes.
 * @details Frees all memory allocated by the array and then frees the
 * array_string_t itself.
*/
void array_strings_destroy(array_strings_t* array);

/**
 * @brief Add a string at the end of the array.
 * @details Appends a string to the end of the array and reallocates memory if
 * neccesary.
 * @returns An error code. 0 means the string was successfully appended.
*/
int array_strings_append(array_strings_t* array, char* str);


/**
 * @brief Get the array's current count.
 * @returns The count of the array.
*/
size_t array_strings_get_count(array_strings_t* array);

/**
 * @brief Get the array's current capacity.
 * @returns The capacity of the array.
*/
size_t array_strings_get_capacity(array_strings_t* array);

/**
 * @brief Copy the string specified by 'index' to 'str_ret'.
 * @details This routine assumes 'index' is a valid index.
*/
void array_strings_get_string(array_strings_t* array, size_t index,
    char* str_ret);


#endif
