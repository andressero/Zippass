// Copyright <2023> <Andrés Serrano>
#include <mpi.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "./array_size.h"
#include "./array_strings.h"
#include "./file_duplicate.h"
#include "./misc.h"
#include "./zippass.h"


#define MAXLENGTH 500  // Max length for strings

/**
 * @brief Send zips to other processes
 * @details Procedure that maps zip paths in 'zips' dinamically for a world of
 * processes of 'process_count' size. This job is divided into four steps:
 * 1. Send zip paths to other processes. To do it dinamically, the process
 * running this procedure receives (waits) the rank of any process and sends a
 * zip to that process. This repeats until all zips are sent.
 * 2. Send a stop condition (empty string) to all processes to stop them from
 * "asking" for more zip.
 * 3. Receive the corresponding password of all zips sent. This process is
 * dynamic too and works the same way as step 1, the difference being that it
 * receives instead of sending.
 * 4. Send a signal to all processes to let them know they can free their data.
 * @param zippass The zippass struct
 * @param zips Array of strings containing zip paths
 * @param passwords Array of string to store passwords
 * @param process_count Size of the world to map the zip paths
 * @returns A status variable. 0 means success
*/
int send_receive_zips(zippass_t* zippass, array_strings_t* zips,
  array_strings_t* passwords, int process_count);

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
 * found. This method now uses MPI and must be executed along send_receive_zips
 * @param zippass The zippass datatype.
 * @param my_process_number The number of the process executing this procedure.
 * @returns 0 if 'passwords' was successfully filled with strings
 * (passwords or empty strings).
 * @see send_receive_zips
*/
int brute_force(zippass_t* zippass, int my_process_number);

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
 * @details Simple routine that frees all memory used by the parameters
 * 'zippass', 'zips' and 'passwords'
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
  MPI_Init(&argc, &argv);

  int my_process_number = -1;
  int process_count = -1;

  if (MPI_Comm_rank(MPI_COMM_WORLD, &my_process_number) == MPI_SUCCESS) {
    if (MPI_Comm_size(MPI_COMM_WORLD, &process_count) == MPI_SUCCESS) {
      // All processes need a zippass_t
      zippass_t *zippass = zippass_create();
      if (zippass) {
        // Array of strings to store zip paths
        array_strings_t *zips = NULL;
        // Array of strings to store passwords
        array_strings_t *passwords = NULL;

        // Only process 0 reads from stdin
        if (my_process_number == 0) {
          zips = array_strings_create(1);
          passwords = array_strings_create(0);
          // Read from stdin to initialize zippass and get zip paths
          read_data(zippass, zips, argc, argv);
        }

        // Variables to initialize zippass for the other processes
        char alphabet[MAXLENGTH] = {0};
        int passwords_max_length = zippass_get_passwords_max_length(zippass);
        int thread_count = zippass_get_thread_count(zippass);
        zippass_get_alphabet(zippass, alphabet);

        // Process 0 broadcasts its zippass's data.
        MPI_Bcast(&alphabet, MAXLENGTH, MPI_CHAR, /*root*/ 0, MPI_COMM_WORLD);
        MPI_Bcast(&passwords_max_length, 1, MPI_INT, /*root*/ 0,
          MPI_COMM_WORLD);
        MPI_Bcast(&thread_count, 1, MPI_INT, /*root*/ 0, MPI_COMM_WORLD);

        // All processes set their zippass with the correct data (even 0,
        // although it's not neccesary)
        zippass_set_alphabet(zippass, alphabet);
        zippass_set_passwords_max_length(zippass, passwords_max_length);
        zippass_set_thread_count(zippass, thread_count);

        if (my_process_number == 0) {
          send_receive_zips(zippass, zips, passwords, process_count);
          print_data(zips, passwords);
          array_strings_destroy(zips);
          array_strings_destroy(passwords);
        } else {
          brute_force(zippass, my_process_number);
        }
        zippass_destroy(zippass);
      }
    }
  }
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

int send_receive_zips(zippass_t* zippass, array_strings_t* zips,
  array_strings_t* passwords, int process_count) {
  // 1. Send zips to processes
  for (size_t zip_index = 0; zip_index < zips->count; ++zip_index) {
    int to_process = -1;
    // Wait to receive a signal from any process
    MPI_Recv(&to_process, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD,
      MPI_STATUS_IGNORE);
    // Send a zip path to that process
    MPI_Send(zips->strings[zip_index], strlen(zips->strings[zip_index]),
      MPI_CHAR, to_process, 0, MPI_COMM_WORLD);
    // Send the index of that path in the array of strings
    MPI_Send(&zip_index, 1, MPI_UNSIGNED_LONG, to_process, 0, MPI_COMM_WORLD);
  }

  // 2. Send stop condition to processes
  for (size_t process_number = 1; process_number < process_count;
    ++process_number) {
    // Send an empty string as the stop condition
    MPI_Send("", 1, MPI_CHAR, process_number, 0, MPI_COMM_WORLD);
    MPI_Send(&process_number, 1, MPI_INT, process_number, 0, MPI_COMM_WORLD);
  }

  // 3. Get passwords from other processes
  array_strings_resize(passwords, zips->count);
  for (size_t index = 0; index < zips->count; ++index) {
    int from_process = -1;
    char password[MAXLENGTH] = {0};
    size_t password_index = -1;

    MPI_Recv(&from_process, 1, MPI_INT, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD,
      MPI_STATUS_IGNORE);
    MPI_Recv(&password, MAXLENGTH, MPI_CHAR, from_process, 2, MPI_COMM_WORLD,
      MPI_STATUS_IGNORE);
    MPI_Recv(&password_index, 1, MPI_UNSIGNED_LONG, from_process, 3,
      MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    array_strings_set(passwords, password, password_index);
  }

  // 4. Let other processes free their data
  bool can_free = true;
  for (size_t process_number = 1; process_number < process_count;
    ++process_number) {
    MPI_Send(&can_free, 1, MPI_C_BOOL, process_number, 0, MPI_COMM_WORLD);
  }
}

int brute_force(zippass_t* zippass, int my_process_number) {
  array_strings_t *my_passwords = array_strings_create(1);
  array_size_t *my_indexes = array_size_create(1);
  if (my_passwords) {
    if (my_indexes) {
      // [ ] Hacer una arreglo dinámico de enteros
      while (true) {
        char my_zip[MAXLENGTH] = {0};
        char password[MAXLENGTH] = {0};
        size_t index = -1;

        MPI_Send(&my_process_number, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        MPI_Recv(&my_zip, MAXLENGTH, MPI_CHAR, 0, MPI_ANY_TAG, MPI_COMM_WORLD,
          MPI_STATUS_IGNORE);
        MPI_Recv(&index, 1, MPI_UNSIGNED_LONG, 0, MPI_ANY_TAG, MPI_COMM_WORLD,
          MPI_STATUS_IGNORE);

        if (strcmp(my_zip, "") == 0) {
          break;
        }

        zippass_brute_force(zippass, my_zip, password);
        array_strings_append(my_passwords, password);
        array_size_append(my_indexes, index);
      }

      for (size_t index = 0; index < my_passwords->count; ++index) {
        MPI_Send(&my_process_number, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);
        MPI_Send(my_passwords->strings[index],
          strlen(my_passwords->strings[index]), MPI_CHAR, 0, 2, MPI_COMM_WORLD);
        MPI_Send(&my_indexes->elements[index], 1, MPI_UNSIGNED_LONG, 0, 3,
          MPI_COMM_WORLD);
      }
      bool can_free = false;
      MPI_Recv(&can_free, 1, MPI_C_BOOL, 0, MPI_ANY_TAG, MPI_COMM_WORLD,
        MPI_STATUS_IGNORE);
      array_size_destroy(my_indexes);
    }
    array_strings_destroy(my_passwords);
  }
}

void print_data(array_strings_t* zips, array_strings_t* passwords) {
  size_t count = zips->count;
  for (size_t i = 0; i < count; ++i) {
    printf("%s %s\n", zips->strings[i], strlen(zips->strings[i]) > 1?
      passwords->strings[i] : "");
  }
}
