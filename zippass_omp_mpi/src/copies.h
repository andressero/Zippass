// Copyright [2023] <Andrés Serrano>

#ifndef COPIES_H
#define COPIES_H

#include <inttypes.h>
#include "array_strings.h"

/**
 * @brief Create 'count' copies of the file in 'source_path'
 * @details This procedure creates a hidden directory called ".copies" that
 * will contain 'count' copies of the file found in 'source path'. Each copy is
 * prefixed with a number, so each copy will have a different filename.
 * @param source_path A path to a file
 * @param count Number of copies to create
 * @returns An array of strings containing the path to all copies.
*/
array_strings_t* create_copies(char* source_path, uint64_t count);

/**
 * @brief Remove all files from 'paths_to_files'. 
 * @details Attempt deleting files using the paths in the array of strings
 * 'copies'. This procedure also deletes the array of strings
 * @param paths_to_files Array that contains the paths to each copy.
*/
void remove_files(array_strings_t* paths_to_files);


#endif
