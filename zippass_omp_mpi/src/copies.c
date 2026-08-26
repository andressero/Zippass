// Copyright [2023] <Andrés Serrano>

#include <libgen.h>
#include <stdlib.h>
#include <inttypes.h>
#include "copies.h"
#include "file_duplicate.h"

array_strings_t* create_copies(char* source_path, uint64_t count) {
  // Create .copies directory if it doesn't exist.
  create_temporary_dir(".copies");
  // Create array of copies
  array_strings_t *copies = array_strings_create(1);
  if (copies) {
    // Allocate a string to temporarily stone the new path
    char *new_path = (char*) calloc(100, sizeof(char));
    if (new_path) {
      // Strip directory from 'path' and leave only the name of the zip archive
      char* filename = basename(source_path);
      // Begin generation of 'count' copies
      for (uint64_t i = 0; i < count; ++i) {
        // Create a path to a copy using 'destination_dir', the index 'i'
        // and 'filename'
        snprintf(new_path, 100, ".copies/%"PRIu64"_%s", i+1, filename);  // NOLINT
        // Attempt file creation
        if (create_temporary_file(source_path, new_path) == EXIT_SUCCESS) {
            // Append path if creation was successful
            array_strings_append(copies, new_path);
        } else {
          // zippass_delete_copies()
          fprintf(stderr, "Error: Couldn't create a zip copy\n");
          // error = something
          return NULL;
        }
      }
      free(new_path);
    } else {
      // error = something
      fprintf(stderr, "Error: Couldn't allocate a string\n");
    }
  } else {
    fprintf(stderr, "Error: Couldn't create an array of strings to store paths"
        "to copies\n");
  }
  return copies;
}

void remove_files(array_strings_t* files) {
  uint64_t count = files->count;
  for (uint64_t i = 0; i < count; ++i) {
    remove_temporary_file(files->strings[i]);
  }
  array_strings_destroy(files);
}

array_strings_t* create_files(uint64_t count) {
  array_strings_t* files = array_strings_create(1);
  create_temporary_dir(".files");
  if (files) {
    char* path = (char*) calloc(100, sizeof(char));
    if (path) {
      for (uint64_t i = 0; i < count; ++i) {
        snprintf(path, 100, ".files/file%"PRIu64, i);  // NOLINT
        FILE* file = fopen(path, "wb");
        fclose(file);
        array_strings_append(files, path);
      }
      free(path);
    }
  }
  return files;
}
