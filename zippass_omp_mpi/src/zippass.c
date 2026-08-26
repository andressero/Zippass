// Copyright <2023> <Andrés Serrano>
#include "zippass.h"
#include <assert.h>
#include <limits.h>
#include <math.h>
#include <omp.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "array_strings.h"
#include "misc.h"
#include "copies.h"


#define ZIP_FILE_OPENED 33
#define ZIP_FILE_NOT_OPENED 34
#define ZIP_FILE_FALSE_POSITIVE 35

typedef struct zip_passwords {
  // The alphabet to generate passwords from.
  char* alphabet;
  // Maximum length a password can be.
  uint64_t passwords_max_length;
  // Total of consumers
  uint64_t thread_count;
}zippass_t;

/// Declaration and documentation of private routines

/**
 * @brief Produce all possible passwords using data of zippass and enqueue
 * them in a shared queue.
 * @details This is the producer routine in the prod-cons pattern.
 * This routine creates passwords using the alphabet from 'zippass'. Passwords
 * generation starts with password of length 1 and finishes with passwords of
 * the maximum length found in 'zippass'. Each generated password is
 * immediately stored in a queue for the consumers to dequeue.
 * @param zippass The zippass datatype
 * @see zippass_generate_password
 * @see zippass_consume_passwords
*/
void zippass_produce_passwords(zippass_t* zippass);

/**
 * @brief Consume passwords from a shared queue and attempt brute force.
 * @details This is the consumer routine in the prod-cons pattern. This routine
 * gets passwords from a queue and attempts brute force.
 * If the password opens the encrypted zip file, it will write the password
 * to a common pointer to a string (called pass_ret). It will also set a
 * boolean to true to stop other threads from attempting brute force.
 * @param data A generic pointer to any kind of datatype.
 * (zippass_thread_data_t in this case)
 * @see zippass_produce_passwords
*/
void* zippass_consume_passwords(void* data);

/**
 * @brief Try to open the first file from 'zip' using 'password'
 * @details This routine assumes the first file from 'zip' is encrypted and
 * will try to open it with 'password'. If the file appears open,
 * it will try to read from it and compare it with the hard-coded code
 * "CI0117-23a" to avoid false positives.
 * @param zip The zip archive in which the zip file is located
 * @param password The password to open the encrypted zip file.
 * @returns A status variable. 33 means the zip successfully opened.
*/
int zippass_fopen(zip_t* zip, char* password);

/// Definition of public routines

zippass_t* zippass_create() {
  // Allocate zippass_t
  zippass_t* zippass = (zippass_t*) calloc(1, sizeof(zippass_t));
  if (zippass) {
    // Allocate alphabet
    zippass->alphabet = (char*) calloc(100, sizeof(char));
    if (zippass->alphabet) {
      // Initialize variables
      zippass->passwords_max_length = 0;
      zippass->thread_count = sysconf(_SC_NPROCESSORS_ONLN);
    } else {
      zippass_destroy(zippass);
      fprintf(stderr, "Error: Couldn't allocate alphabet string\n");
      return NULL;
    }
  } else {
    fprintf(stderr, "Error: Couldn't allocate zippass_t datatype\n");
    return NULL;
  }
  return zippass;
}

void zippass_destroy(zippass_t* zippass) {
  free(zippass->alphabet);
  free(zippass);
}

int zippass_brute_force(zippass_t* zippass, char* zip_path, char* pass_ret) {
  char* alphabet = zippass->alphabet;
  size_t alphabet_length = strlen(alphabet);
  size_t max_length = zippass->passwords_max_length;
  bool brute_forced = false;

  uint64_t password_count = geometric_sum(alphabet_length, 1, max_length);
  uint64_t* counts = (uint64_t*) calloc(max_length, sizeof(uint64_t));
  if (counts) {
    for (uint64_t i = 0; i < max_length; ++i) {
      counts[i] = (uint64_t) pow(alphabet_length, i+1);
    }

    array_strings_t* copies = create_copies(zip_path, zippass->thread_count);
    if (copies) {
      #pragma omp parallel default(none) shared(copies, password_count, \
        max_length, counts, alphabet, alphabet_length, pass_ret, brute_forced)
      {  // NOLINT
        zip_t* my_zip;
        #pragma omp critical(can_open_zip)
        {
          my_zip = zip_open(copies->strings[omp_get_thread_num()], 0, NULL);
        }

        if (my_zip) {
          char my_password[100] = {0};
          #pragma omp for
          for (uint64_t i = 0; i < password_count; ++i) {
            size_t password_length = get_length(i, counts, max_length);
            generate_string(i, password_length, alphabet, alphabet_length,
              my_password);
            if (zippass_fopen(my_zip, my_password) == ZIP_FILE_OPENED) {
              snprintf(pass_ret, strlen(my_password)+1, "%s", my_password);
              brute_forced = true;
            }
          }
          zip_close(my_zip);
        }
      }  // End parallel
      remove_files(copies);
    }
    free(counts);
  }

  if (!brute_forced) {
    snprintf(pass_ret, strlen("")+1, "%s", "");
  }
}

void zippass_set_alphabet(zippass_t* zippass, char* alphabet) {
  snprintf(zippass->alphabet, strlen(alphabet)+1, "%s", alphabet);
}

void zippass_set_passwords_max_length(zippass_t* zippass, uint64_t length) {
  zippass->passwords_max_length = length;
}

void zippass_set_thread_count(zippass_t* zippass, uint64_t thread_count) {
  zippass->thread_count = thread_count;
}

void zippass_get_alphabet(zippass_t* zippass, char* retval) {
  snprintf(retval, strlen(zippass->alphabet)+1, "%s", zippass->alphabet);
}

uint64_t zippass_get_passwords_max_length(zippass_t* zippass) {
  return zippass->passwords_max_length;
}

uint64_t zippass_get_thread_count(zippass_t* zippass) {
  return zippass->thread_count;
}

/// Definition of private routines

int zippass_fopen(zip_t* zip, char* password) {
  assert(zip);
  assert(password);
  int status = ZIP_FILE_NOT_OPENED;
  // Code used to check if the file was actually opened.
  char code[] = "CI0117-23a";
  // Buffer to read from zip file and compare text with the code.
  char buffer[11] = {0};

  // Try to open encrypted zip file with 'password'
  zip_file_t *zip_file = zip_fopen_index_encrypted(zip, 0, 0, password);
  // If the zip file opened
  if (zip_file) {
    // Read from zip_file and store to 'buffer'
    if (zip_fread(zip_file, buffer, 11) > 0) {
      // Compare 'code' with 'buffer'
      if (strcmp(code, buffer) == 0) {
        // printf("Found password: %s\n%s %s\n", password, code, buffer);
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
