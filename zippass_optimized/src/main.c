// Copyright <2023> <Andrés Serrano>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "./array_strings.h"
#include "./misc.h"
#include "./file_duplicate.h"
#include "./zippass.h"

#define MAXLENGTH 500  // Max length for strings

/**
 * @brief Read from stdin and fill variables from both 'zippass' and
 * 'zips' datatypes.
 * @details Reads from stdin until finding EOF. This routine expects to be
 * reading from a batch file which contains data for "zippass's" variables
 * (alphabet and passwords_max_lenght) and paths to zip files (which will be
 * stored in the array of strings 'zips').
 * @param zippass The zippass datatype.
 * @param zips Array of strings in which paths will be written to.
 * @param argc main's argument count.
 * @param argv main's argument vector.
 * @returns A status variable that informs if data could be successfully read
 * and stored.
 * 0 means success.
*/
int read_data(zippass_t* zippass, array_strings_t* zips, int argc,
    char* argv[]);

/**
 * @brief Brute force all zips in 'zips' and store passwords in 'passwords'.
 * @details This routine uses zippass_brute_force on all zip paths found in
 * array_strings_t *zips and appends the password in
 * array_strings_t *passwords or an empty string if the password couldn't be
 * found.
 * @param zippass The zippass datatype.
 * @param zips Array of strings with paths to zip archives.
 * @param passwords Array of strings in which passwords will be written to.
 * @returns 0 if 'passwords' was successfully filled with strings
 * (passwords or empty strings).
*/
int brute_force(zippass_t* zippass, array_strings_t* zips,
    array_strings_t* passwords);

/**
 * @brief Print all paths to zips alongside their password (if found).
 * @param zips Array of strings containing zip paths.
 * @param passwords Array of strings containing passwords.
*/
void print_data(array_strings_t* zips, array_strings_t* passwords);

/**
 * @brief Allocate all data structures used in main.
 * @details Simple routine that tries to allocate memory for all parameters.
 * The reason why this procedure was created was to reduce the amount of nested
 * if statements the main function had.
 * @param zippass Address to a zippass datatype.
 * @param zips Address to an array of strings.
 * @param passwords Address to an array of strings.
 * @returns An error code. 0 means success.
 * @see free_data_structures
*/
int allocate_data_structures(zippass_t** zippass, array_strings_t** zips,
    array_strings_t** passwords);

/**
 * @brief Free all data structures used in main.
 * @details Simple routine that frees all memory used by the parameters 'zippass', 'zips' and 'passwords'
 * @param zippass Address to a zippass datatype.
 * @param zips Address to an array of strings.
 * @param passwords Address to an array of strings.
 * @see allocate_data_structures
*/
void free_data_structures(zippass_t** zippass, array_strings_t** zips,
    array_strings_t** passwords);

/**
 * @brief Start program execution.
 *
 * @return Status code to the operating system, 0 means success.
 */

int main(int argc, char* argv[]) {
  int error = EXIT_SUCCESS;
  zippass_t *zippass = NULL;
  array_strings_t *zips = NULL;
  array_strings_t *passwords = NULL;

  // Allocate data
  if ((error = allocate_data_structures(&zippass, &zips, &passwords))
      == EXIT_SUCCESS) {
    // Read from stdin and store data in 'zippass' and 'zips' datatypes
    if ((error = read_data(zippass, zips, argc, argv)) == EXIT_SUCCESS) {
      // Attempt brute force on each zip and save results in 'passwords'
      if ((error = brute_force(zippass, zips, passwords)) == EXIT_SUCCESS) {
        // Print zip paths alongside their passwords (if found)
        print_data(zips, passwords);
      } else {
        fprintf(stderr, "Error: Couldn't store a password\n");
      }
    } else {
      fprintf(stderr, "Error: Couldn't read data from file\n");
    }

    free_data_structures(&zippass, &zips, &passwords);
  }
  return error;
}

int read_data(zippass_t* zippass, array_strings_t* zips, int argc,
    char* argv[]) {
  int error = EXIT_SUCCESS;
  char line[MAXLENGTH] = {0};

  // 1. Read first line: the alphabet.
  error = get_line(line, MAXLENGTH, stdin) > 0? EXIT_SUCCESS: EXIT_FAILURE;
  if (error == EXIT_SUCCESS) {
    // Remove \n from 'line'.
    // Adapted from https://stackoverflow.com/questions/2693776/removing-trailing-newline-character-from-fgets-input
    line[strcspn(line, "\n")] = 0;
    // Store the alphabet.
    zippass_set_alphabet(zippass, line);
    // 2. Read second line: the password's max length.
    error = get_line(line, MAXLENGTH, stdin) > 0? EXIT_SUCCESS: EXIT_FAILURE;
    if (error == EXIT_SUCCESS) {
      // Convert the line to an integer.
      int64_t length = atoi(line);
      // Check that the length is greater than 0
      // Note: 'length' could be 0 if conversion was unsuccessful
      if (length > 0) {
        zippass_get_alphabet(zippass, line);
        // Make sure the specified length is less than the length of the
        // alphabet.
        if (length < (int64_t)strlen(line)) {
          // Store the password's max length
          zippass_set_passwords_max_length(zippass, length);
          // 3. Read third line: the blank line
          error =
              get_line(line, MAXLENGTH, stdin) > 0? EXIT_SUCCESS: EXIT_FAILURE;
          if (error == EXIT_SUCCESS) {
            // 4. Read the rest of the lines: zip paths
            while (get_line(line, MAXLENGTH, stdin) > 0) {
              // Remove \n before appending.
              line[strcspn(line, "\n")] = 0;
              // Store zip path.
              if ((error = array_strings_append(zips, line)) == EXIT_FAILURE) {
                fprintf(stderr, "Error: Couldn't store a zip path\n");
                break;
              }
            }

            if (argc == 2) {
              uint64_t thread_count = 0;
              if (sscanf(argv[1], "%"PRIu64, &thread_count) == 1) {
                zippass_set_thread_count(zippass, thread_count);
              } else {
              }
            }
          } else {
            fprintf(stderr, "Error: Couldn't read the blank line\n");
          }
        } else {
          fprintf(stderr, "Error: The length of the password cannot be"
              " greater than the length of the alphabet.\n");
        }
      } else {
        fprintf(stderr, "Error: Couldn't store password's maximum length.\nThe"
            " length you specified might be too long or is a negative "
            "number.\n");
      }

    } else {
      fprintf(stderr,
        "Error: Couldn't read password's maximum length from stdin\n");
    }
  } else {
    fprintf(stderr, "Error: Couldn't read alphabet from stdin\n");
  }
  return error;
}

int brute_force(zippass_t* zippass, array_strings_t* zips,
    array_strings_t* passwords) {
  int error = EXIT_SUCCESS;
  char password[MAXLENGTH] = {0};
  char zip[MAXLENGTH] = {0};
  size_t count = array_strings_get_count(zips);

  // Attempt brute force on all zips in 'zips' and store the password in the
  // array
  create_temporary_dir(".copies");

  for (size_t i = 0; i < count; ++i) {
    array_strings_get_string(zips, i, zip);
    zippass_brute_force(zippass, zip, password);
    // Append password (empty or not) to the array of passwords
    if ((error = array_strings_append(passwords, password)) != EXIT_SUCCESS) {
      error = EXIT_FAILURE;
      break;
    }
  }
  remove_temporary_dir(".copies");
  return error;
}

void print_data(array_strings_t* zips, array_strings_t* passwords) {
  char password[MAXLENGTH];
  char zip[MAXLENGTH];
  size_t count = array_strings_get_count(zips);
  for (size_t i = 0; i < count; ++i) {
    array_strings_get_string(zips, i, zip);
    array_strings_get_string(passwords, i, password);
    printf("%s %s\n", zip, strlen(password) > 1? password : "");
  }
}

int allocate_data_structures(zippass_t** zippass, array_strings_t** zips,
    array_strings_t** passwords) {
  int error = EXIT_FAILURE;
  *zippass = zippass_create();
  if (zippass) {
    *zips = array_strings_create();
    if (zips) {
      *passwords = array_strings_create();
      if (passwords) {
        error = EXIT_SUCCESS;
      } else {
        fprintf(stderr, "Error: Couldn't allocate array of strings for "
        "passwords\n");
      }
    } else {
      fprintf(stderr, "Error: Couldn't allocate array of strings for zip "
      "paths\n");
    }
  } else {
    fprintf(stderr, "Error: Couldn't allocate zippass_t datatype\n");
  }

  if (error == EXIT_FAILURE) {
    free_data_structures(zippass, zips, passwords);
  }

  return error;
}

void free_data_structures(zippass_t** zippass, array_strings_t** zips,
    array_strings_t** passwords) {
  if (*zippass) {
    zippass_destroy(*zippass);
  }
  if (*zips) {
    array_strings_destroy(*zips);
  }
  if (*passwords) {
    array_strings_destroy(*passwords);
  }
}
