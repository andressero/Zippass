// Copyright <2023> <Andrés Serrano>
#ifndef MISC_H
#define MISC_H

#include <stdio.h>
#include <inttypes.h>

/**
 * @brief Read from 'file' and write input to 'line' until finding a newline,
 * EOF or reaching 'max_capacity'.
 * @details This routine is adapted from an example shown in the first chapter
 * of the famous book "The C programming language" authored by
 * Brian W. Kernighan and Dennis M. Ritchie.
 * @param line A string to return the line read
 * @param max_capacity The maximun length that will be written to 'line'
 * @param file The file to read from.
 * @returns The length of the line. A length of 0 means first character read
 * was EOF.
*/
uint64_t get_line(char line[], size_t max_capacity, FILE* file);

/**
 * @brief Get the minimum between three numbers
 * @details This function returns the smallest number of all parameters. It is
 * used in conjunction with the map method. The names of the variable follow
 * the context of mapping data statically by blocks.
 * @param worker
 * @param data
 * @param worker_count
 * @returns The minimum number
 * @see map
*/
uint64_t minimum(uint64_t worker, uint64_t data, uint64_t worker_count);

/**
 * @brief Map data statically by block
 * @details This function returns the data 'worker' will need to
 * handle, based on the total amount of data ('data') there is, and the total of
 * workers ('worker_count') to distribute the data to. Keep in mind that in
 * order to get the full range for one worker, this function needs to be called
 * twice. Calling map(4,x,w) gives you the position of data worker 4 will start
 * at; calling map((4+1),x,w) gives you the position of data worker 4 will
 * finish at.
 * @param worker Data for this worker
 * @param data The total of data that needs to be mapped
 * @param worker_count The total of workers available.
 * 
*/
uint64_t map(uint64_t worker, uint64_t data, uint64_t worker_count);

/**
 * @brief Generate a string of length 'length' using 'alphabet' and 'index'.
 * Return password in 'retval'.
 * @details This routine creates a string, character by character, in a for loop
 * that starts at index 0 and ends at index 'length'. To get characters from the
 * alphabet, it calculates indexes operating 'index' modulo 'alphabet_length'
 * and then dividing 'index' by 'alphabet_length'.
 * @param index An index to generate the string number 'index'.
 * @param length Desired length of the password
 * @param alphabet An alphabet to generate a password from.
 * @param alphabet_length Length of the alphabet
 * @param retval Pointer to write the password to.
*/
void generate_string(uint64_t index, uint64_t length, char* alphabet,
    uint64_t alphabet_length, char* retval);

/**
 * @brief Return the partial geometric sum of base 'base', from 'first_exponent'
 * to 'last_exponent'.
 * @param base The base of the geometric sum
 * @param first_exponent The 'i' this sum will start at
 * @param last_exponent The 'n', where the geometric sum will stop summing.
*/
uint64_t geometric_sum(uint64_t base, uint64_t first_exponent,
    uint64_t last_exponent);

/**
 * @brief Get the length of a string given from an index. 
 * @details This procedure is executed along generate_string, which requires a
 * length to create a string, but it's not part of it. This function is useful
 * if the length is unknown. To work properly, this procedure makes use of the
 * array of integer 'counts', which must be an array of counts of all possible
 * combinations that can be made from an alphabet with a given length.
 * @param index The index to get the length from
 * @param counts Array of counts of different lengths
 * @param counts_size Size of the array 'counts'
*/
uint64_t get_length(int64_t index, uint64_t* counts, uint64_t counts_size);

#endif
