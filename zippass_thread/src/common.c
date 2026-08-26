// Copyright <2023> <Andrés Serrano>
#include "common.h"
#include <stdlib.h>

// Adapted from the Kernighan and Ritchie's book "The C programming language"
// It also works as a getline that doesn't depend on _GNU_SOURCE
uint64_t get_line(char s[], size_t max) {
  int c = 0;
  size_t i = 0;
  for (i = 0; i < max-1 && (c = getchar()) != EOF && c != '\n'; ++i) {
    s[i] = c;
  }
  if (c == '\n') {
    s[i++] = c;
  }
  s[i] = '\0';
  return i;
}
