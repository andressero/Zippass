// Copyright [2023] <Bryan Ulate>

# ifndef FILEDUPLICATE_H
# define FILEDUPLICATE_H

# include <stdio.h>
# include <stdlib.h>
# include <sys/stat.h>
# include <unistd.h>

/**
 * @brief Create a directory located in 'path'
 * @details Create the directory if it doesn't exist. Check that the directory
 * doesn't exist by requesting its attributes.
 * @param path A path to the directory
 * @returns 0 if creation was successful or -1 if it was already created
*/
int create_temporary_dir(char* path);

/**
 * @brief Create a copy of the file in 'path_original_file' to 'tmp_file_path'.
 * @details This routine creates a new file in 'tmp_file_path' and writes in 
 * it, all contents of the file located in 'path_original_file', effectively
 * creating a copy.
 * @param path_original_file Path to file to read from
 * @param tmp_file_path Path to file to write to
 * @returns A status code. 0 means success
*/
int create_temporary_file(char* path_original_file, char* tmp_file_path);

/**
 * @brief Remove the directory located in 'path'
 * @details Remove the directory if it exists. Check that the directory exists
 * by requesting its attributes.
 * @param path A path to the directory
 * @returns A status code. 0 means success
*/
int remove_temporary_dir(char* path);

/**
 * @brief Remove the file located in 'tmp_file_path'
 * @details Remove the file if it exists. Check that the file exists by
 * requesting its attributes.
 * @param tmp_file_path A path to the file
 * @return A status code. 0 means success
*/
int remove_temporary_file(char* tmp_file_path);


#endif
