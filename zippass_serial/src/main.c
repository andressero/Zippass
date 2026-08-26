// Copyright <2023> <Andrés Serrano>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "./array_strings.h"
#include "./common.h"
#include "./zippass.h"

#define MAXLENGTH 500  // Max length for strings

/**
 * @brief Read from stdin and fill variables from both 'zippass' and
 * 'zips' datatypes.
 * @details Reads from stdin until finding EOF. This routine expects to be
 * reading from a batch file which contains data for "zippass's" variables
 * (alphabet and passwords_max_lenght) and paths to zip files (which will be
 * stored in the array of strings 'zips').
 * @returns A status variable that informs if data could be successfully read
 * and stored.
 * 0 means success.
*/
int read_data(zippass_t* zippass, array_strings_t* zips);

/**
 * @brief Brute force all zips in 'zips' and store passwords in 'passwords'.
 * @details This routine uses zippass_brute_force on all zip paths found in
 * array_strings_t *zips and appends the password in
 * array_strings_t *passwords or an empty string if the password couldn't be
 * found.
 * @returns 0 if 'passwords' was successfully filled with strings
 * (passwords or empty strings).
*/
int brute_force(zippass_t* zippass, array_strings_t* zips,
  array_strings_t* passwords);

/**
 * @brief Print all paths to zips alongside their password (if found).
*/
void print_data(array_strings_t* zips, array_strings_t* passwords);


/**
 * @brief Start program execution.
 *
 * @return Status code to the operating system, 0 means success.
 */
int main(void) {
  int error = EXIT_SUCCESS;
  // Start allocating datatypes
  zippass_t *zippass = zippass_create();
  array_strings_t *zips = array_strings_create();
  array_strings_t *passwords = array_strings_create();

  // If everything was successfully allocated
  if (zippass) {
    if (zips) {
      if (passwords) {
        // Read from stdin and store data in 'zippass' and 'zips' datatypes
        error = read_data(zippass, zips);
        // If data loading was successful
        if (error == EXIT_SUCCESS) {
          // Attempt brute force on each zip and store each password in the
          // array 'passwords'.
          error = brute_force(zippass, zips, passwords);
          // If there was no problems storing passwords
          if (error == EXIT_SUCCESS) {
            // Print zip paths alongside their passwords (if found)
            print_data(zips, passwords);
          } else {
            fprintf(stderr, "Error: Couldn't store a password\n");
          }
        } else {
          fprintf(stderr, "Error: Couldn't read data from batch\n");
        }
          // Finish freeing allocated memory
          array_strings_destroy(passwords);
      } else {
        fprintf(stderr, "Error: Couldn't allocate passwords\n");
        error = 11;
      }
      array_strings_destroy(zips);
    } else {
      fprintf(stderr, "Error: Couldn't allocate zips paths\n");
      error = 12;
    }
    zippass_destroy(zippass);
  } else {
    fprintf(stderr, "Error: Couldn't allocate zippass_t datatype\n");
    error = 13;
  }
  return error;
}

int read_data(zippass_t* zippass, array_strings_t* zips) {
  int error = EXIT_SUCCESS;
  char line[MAXLENGTH] = {0};

  // 1. Read first line: the alphabet.
  error = get_line(line, MAXLENGTH) > 0? EXIT_SUCCESS: EXIT_FAILURE;
  if (error == EXIT_SUCCESS) {
    // Remove \n from 'line'.
    // Adapted from https://stackoverflow.com/questions/2693776/removing-trailing-newline-character-from-fgets-input
    line[strcspn(line, "\n")] = 0;
    // Store the alphabet.
    zippass_set_alphabet(zippass, line);
    // 2. Read second line: the password's max length.
    error = get_line(line, MAXLENGTH) > 0? EXIT_SUCCESS: EXIT_FAILURE;
    if (error == EXIT_SUCCESS) {
      // Convert the line to an integer.
      int64_t length = atoi(line);
      // Check that the length is greater than 0
      // 'length' could be 0 if conversion was unsuccessful
      if (length > 0) {
        zippass_get_alphabet(zippass, line);
        // printf("%"PRIu64" %zu\n", length, strlen(line));
        // Make sure the specified length is less than the length of the
        // alphabet.
        if (length < (int64_t)strlen(line)) {
          // Store the password's max length
          zippass_set_passwords_max_length(zippass, length);
          // 3. Read third line: the blank line
          error = get_line(line, MAXLENGTH) > 0? EXIT_SUCCESS: EXIT_FAILURE;
          if (error == EXIT_SUCCESS) {
            // 4. Read the rest of the lines: zip paths
            while (get_line(line, MAXLENGTH) > 0) {
              // Remove \n before appending.
              line[strcspn(line, "\n")] = 0;
              // Store zip path.
              if ((error = array_strings_append(zips, line)) == EXIT_FAILURE) {
                fprintf(stderr, "Error: Couldn't store a zip path\n");
                break;
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
            " length you specified could be too long or is a negative "
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
  char password[MAXLENGTH];
  char zip[MAXLENGTH];
  size_t count = array_strings_get_count(zips);

  // Attempt brute force on all zips in 'zips' and store the password in the
  // array
  for (size_t i = 0; i < count; ++i) {
    array_strings_get_string(zips, i, zip);
    zippass_brute_force(zippass, zip, password);
    // Append password (empty or not) to the array of passwords
    // printf("Appending password: %s\n\n", password);
    if ((error = array_strings_append(passwords, password)) != EXIT_SUCCESS) {
      error = EXIT_FAILURE;
      break;
    }
  }
  return error;
}

void print_data(array_strings_t* zips, array_strings_t* passwords) {
    char password[MAXLENGTH];
  char zip[MAXLENGTH];
  size_t count = array_strings_get_count(zips);
  for (size_t i = 0; i < count; ++i) {
    array_strings_get_string(zips, i, zip);
    array_strings_get_string(passwords, i, password);
    if (strlen(password) > 1) {
      printf("%s %s\n", zip, password);
    } else {
      printf("%s\n", zip);
    }
  }
}
