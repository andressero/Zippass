// Copyright <2023> <Andrés Serrano>
#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <inttypes.h>

/**
 * @brief Read from stdin and write input to 'line' until finding a newline,
 * EOF or reaching 'max_capacity'.
 * @details This routine is adapted from an example shown in the first chapter
 * of the famous book "The C programming language" authored by
 * Brian W. Kernighan and Dennis M. Ritchie.
 * @param line A string to return the line read
 * @param max_capacity The maximun length that will be written to 'line'
 * @returns The length of the line. A length of 0 means first character read
 * was EOF.
*/
uint64_t get_line(char line[], size_t max_capacity);

#endif
