// Copyright <2023> <Andrés Serrano>
#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "zippass.h"

#define ZIP_FILE_OPENED 33
#define ZIP_FILE_NOT_OPENED 34
#define ZIP_FILE_FALSE_POSITIVE 35

typedef struct zip_passwords {
  char* alphabet;
  uint64_t passwords_max_length;
}zippass_t;

int zippass_fopen(zip_t* zip, char* password);
void zippass_next_password(uint64_t* indexes, uint64_t array_length,
    uint64_t max);
void zippass_get_password(char* alphabet, char* password,
    uint64_t password_length, uint64_t* indexes);


zippass_t* zippass_create() {
  // Allocate zippass_t
  zippass_t* zippass = (zippass_t*) calloc(1, sizeof(zippass_t));
  if (zippass) {
    // Allocate alphabet
    zippass->alphabet = (char*) calloc(100, sizeof(char));
    if (zippass->alphabet) {
      // Initialize passwords_max_length
      zippass->passwords_max_length = 0;
      return zippass;
    } else {
      fprintf(stderr, "Error: Couldn't allocate alphabet string\n");
      return NULL;
    }
  } else {
    fprintf(stderr, "Error: Couldn't allocate zippass_t datatype\n");
    return NULL;
  }
}

void zippass_destroy(zippass_t* zippass) {
  free(zippass->alphabet);
  free(zippass);
}

int zippass_brute_force(zippass_t* zippass, char* zip_path, char* pass_ret) {
  assert(zippass);
  assert(zip_path);
  assert(strlen(zip_path) > 0);

  int status = NOT_BRUTE_FORCED;
  // Bool that is set to true if one of the passwords opened the file.
  bool opened = false;
  // Try to open the zip archive.
  zip_t* zip = zip_open(zip_path, /*flags*/0, /*errorp*/NULL);
  if (zip) {
    // Prepare for password generation.
    uint64_t max_length = zippass->passwords_max_length;
    uint64_t alphabet_length = (uint64_t) strlen(zippass->alphabet);

    // String to store a password.
    char password[100] = {0};
    // This first 'if' statement is used to avoid a warning that seems to be a
    // bug.
    // Adapted from: https://gcc.gnu.org/bugzilla//show_bug.cgi?id=85783
    if (max_length < PTRDIFF_MAX) {
      // Allocate an array of ints to be used as indexes and get passwords from
      // that
      uint64_t *indexes = (uint64_t*) calloc(max_length, sizeof(uint64_t));
      if (indexes) {
        // Begin password generation
        // External 'for' loops through all lengths of password
        for (uint64_t i = 0; i < max_length && !opened; ++i) {
          uint64_t count = (uint64_t) pow(alphabet_length, (i+1));
          // Internal 'for' generates passwords of length i+1
          for (uint64_t j = 0; j < count && !opened; ++j) {
            // Store password in 'password'
            zippass_get_password(zippass->alphabet, password, i+1, indexes);
            // Try to open the encrypted file using the password
            // If the password successfully opened the file
            if (zippass_fopen(zip, password) == ZIP_FILE_OPENED) {
              // Copy it to 'pass_ret'
              snprintf(pass_ret, strlen(password)+1, "%s", password);
              status = BRUTE_FORCED;
              opened = true;
            } else {
              // Change indexes to get the next password
              zippass_next_password(indexes, i+1, alphabet_length-1);
            }
          }
        }
        free(indexes);
      } else {
        fprintf(stderr, "Error: Couldn't allocate array of indexes\n");
        status = 14;
      }
    } else {
      fprintf(stderr, "Error: The password's max length is too long\n");
    }

    zip_close(zip);
  } else {
    fprintf(stderr, "Error: Couldn't open: %s", zip_path);
    status = ZIP_NOT_OPENED;
  }
  if (!opened) {
    // Write an empty string to 'pass_ret' if the password wasn't found.
    snprintf(pass_ret, strlen("")+1, "%s", "");
  }
  return status;
}

void zippass_set_alphabet(zippass_t* zippass, char* alphabet) {
  snprintf(zippass->alphabet, strlen(alphabet)+1, "%s", alphabet);
}

void zippass_set_passwords_max_length(zippass_t* zippass, uint64_t length) {
  zippass->passwords_max_length = length;
}

void zippass_get_alphabet(zippass_t* zippass, char* retval) {
  snprintf(retval, strlen(zippass->alphabet)+1, "%s", zippass->alphabet);
}

uint64_t zippass_get_passwords_max_length(zippass_t* zippass) {
  return zippass->passwords_max_length;
}

// Private routines

/**
 * @brief Get a password from 'alphabet' using the indexes from 'indexes'.
 * @details This routine uses a for loop to travel the 'indexes' array and the
 * password (both should have the same length). The password is created
 * interpreting each element of 'indexes' as a valid index to get a character
 * from the string 'alphabet'.
*/
void zippass_get_password(char* alphabet, char* password,
  uint64_t password_length, uint64_t* indexes) {
  uint64_t i = 0;
  for (i = 0; i < password_length; ++i) {
    password[i] = alphabet[indexes[i]];
  }
  password[i+1] = '\0';
}

/**
 * @brief Increment one element of 'indexes' by one.
 * @details If each slot of 'indexes' is seen as a digit and the array is seen
 * as a whole number, then this routines simply increments that number by one.
 * This routine is necessary since 'indexes' is as big as the password's length
 * and sometimes, as long as the alphabet's, which could easily be greater
 * than 10. So, this routine ensures each slot of 'indexes' (each so-called
 * digit) is properly incremented or set to 0 when reaching 'max' (and not when
 * reaching 9 like the usual decimal system).
*/
void zippass_next_password(uint64_t* indexes, uint64_t array_length,
    uint64_t max) {
  // Travel the array "backwards", starting from the "leftmost digit" and
  // attempt incrementing from there as if 'indexes'++ was a thing.
  for (int64_t i = array_length-1; i >=0; --i) {
    // Set indexes[i] to 0 if it reached 'max'
    if (indexes[i] == max) {
      indexes[i] = 0;
    } else {
      // Increment one slot once and then return.
      ++indexes[i];
      break;
    }
  }
}

/**
 * @brief Try to open the first file from 'zip' using 'password'
 * @details This routine assumes the first file from 'zip' is encrypted and
 * will try to open it with 'password'. If the file seems to have been opened,
 * it will try to read from it and compare it with the hard-coded code
 * "CI0117-23a" to avoid false positives.
 * @returns A status variable. 33 means the zip successfully opened.
*/
int zippass_fopen(zip_t* zip, char* password) {
  assert(zip);
  assert(password);
  int status = ZIP_FILE_NOT_OPENED;
  // Code used to check if the file was actually opened.
  char code[] = "CI0117-23a";
  // Buffer to read from zip file and compare with the code.

  char buffer[11];
  for (int i = 0; i < 11; ++i) {
    buffer[i] = 0;
  }

  zip_file_t *zip_file = zip_fopen_index_encrypted(zip, 0, 0, password);
  // If the zip file opened
  if (zip_file) {
    // And reading was successful
    if (zip_fread(zip_file, buffer, 11) > 0) {
      if (strcmp(code, buffer) == 0) {
        status = ZIP_FILE_OPENED;
      } else {
        // buffer and code are not equal
        status = ZIP_FILE_FALSE_POSITIVE;
      }
    } else {
      // Couldn't read from zip file
      status = ZIP_FILE_NOT_OPENED;
    }
    zip_fclose(zip_file);
  }
  return status;
}
