// Copyright [2023] <Andrés Serrano>

#include "zippass_threads.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

pthread_t* zippass_create_threads(zippass_thread_data_t* threads_data,
    uint64_t count, void*(*subroutine)(void*)) {
  pthread_t* threads = (pthread_t*) calloc(count, sizeof(pthread_t));
  if (threads) {
    for (size_t index = 0; index < count; ++index) {
      if (pthread_create(&threads[index], /*attr*/ NULL, subroutine,
          &threads_data[index]) == EXIT_SUCCESS) {
      } else {
        fprintf(stderr, "error: could not create thread %zu\n", index);
        zippass_join_threads(index, threads);
        return NULL;
      }
    }
  }
  return threads;
}

int zippass_join_threads(size_t count, pthread_t* threads) {
  int error = EXIT_SUCCESS;
  for (size_t index = 0; index < count; ++index) {
    // todo: sum could not be right
    error += pthread_join(threads[index], /*value_ptr*/ NULL);
  }
  free(threads);
  return error;
}

zippass_thread_data_t* zippass_create_threads_data(zippass_t* zippass,
    array_strings_t* copies, char* password_return) {
  uint64_t count = zippass_get_thread_count(zippass);
  zippass_thread_data_t* threads_data = (zippass_thread_data_t*)
      calloc(count, sizeof(zippass_thread_data_t));
  if (threads_data) {
    for (uint64_t i = 0; i < count; ++i) {
      threads_data[i].my_thread_number = i;
      threads_data[i].my_zip_path_copy = calloc(200, sizeof(char));
      // array_strings_get_string(copies, i, threads_data[i].my_zip_path_copy);
      snprintf(threads_data[i].my_zip_path_copy, strlen(copies->strings[i])+1,
        "%s", copies->strings[i]);
      threads_data[i].password_return = password_return;
      threads_data[i].zippass = zippass;
    }
  } else {
    fprintf(stderr, "Error: Couldn't create thread data\n");
  }
  return threads_data;
}

void zippass_destroy_threads_data(zippass_thread_data_t* threads_data,
    uint64_t count) {
  // free my_zip_path_copy
    for (uint64_t i = 0; i < count; ++i) {
      free(threads_data[i].my_zip_path_copy);
    }
    free(threads_data);
}

