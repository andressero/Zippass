// Copyright <2023> <Andrés Serrano>
#include "misc.h"
#include <stdlib.h>
#include <math.h>

// Adapted from the Kernighan and Ritchie's book "The C programming language"
// It also works as a getline that doesn't depend on _GNU_SOURCE
uint64_t get_line(char s[], size_t max, FILE* file) {
  int c = 0;
  size_t i = 0;
  for (i = 0; i < max-1 && (c = getc(file)) != EOF && c != '\n'; ++i) {
    s[i] = c;
  }
  if (c == '\n') {
    s[i++] = c;
  }
  s[i] = '\0';
  return i;
}

uint64_t minimum(uint64_t i, uint64_t d,
    uint64_t w) {
    uint64_t dw = d % w;
    if (i < dw) {
        return i;
    } else {
        return dw;
    }
}

uint64_t map(uint64_t i, uint64_t d,
    uint64_t w) {
    return i * floor(d/w) +
        minimum(i, d, w);
}

void generate_string(uint64_t index, uint64_t length, char* alphabet,
    uint64_t alphabet_length, char* retval) {
  uint64_t modulo = 0;
  uint64_t i = 0;

  for (i = 0; i < length; ++i) {
    // Get a valid index (for alphabet) using modulo
    modulo = index % alphabet_length;
    // Write in password_return the character from alphabet at index 'modulo'
    retval[i] = alphabet[modulo];
    // Divide index by base to get it ready for next modulo operation
    index = index / alphabet_length;
  }

  retval[i] = '\0';
}

uint64_t geometric_sum(uint64_t x, uint64_t first_k, uint64_t last_k) {
  uint64_t sum = 0;

  for (uint64_t k = first_k; k <= last_k; ++k) {
    sum += (uint64_t) pow(x, k);
  }

  return sum;
}
