// Copyright <2023> <Andrés Serrano>

#ifndef ARRAY_STRINGS_H
#define ARRAY_STRINGS_H

#include <stddef.h>

/**
 * @brief A dynamic array of strings
 * @details This struct contains an array of strings (char**) and two variables
 * to keep count of how many strings it currently has (count) and how many
 * strings it can currently hold (capacity). Thanks to those two variables, this
 * struct will automatically increase the capacity of the array of strings, if
 * neccesary.
*/
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
 * @param array The array of strings
*/
void array_strings_destroy(array_strings_t* array);

/**
 * @brief Add a string to the end of the array.
 * @details Appends a string to the end of the array and reallocates memory if
 * neccesary.
 * @param array The array of strings
 * @param str The string that is going to be appended
 * @returns An error code. 0 means the string was successfully appended.
*/
int array_strings_append(array_strings_t* array, char* str);

// Gets

/**
 * @brief Get the array's current count.
 * @param array The array of strings
 * @returns The count of the array.
*/
size_t array_strings_get_count(array_strings_t* array);

/**
 * @brief Get the array's current capacity.
 * @param array The array of strings
 * @returns The capacity of the array.
*/
size_t array_strings_get_capacity(array_strings_t* array);

/**
 * @brief Copy the string specified by 'index' to 'str_ret'.
 * @param array The array of strings
 * @param index Position of the array that will be returned
 * @param str_ret Pointer in which the string will be copied.
 * @details This routine assumes 'index' is a valid index.
*/
void array_strings_get_string(array_strings_t* array, size_t index,
    char* str_ret);


#endif
