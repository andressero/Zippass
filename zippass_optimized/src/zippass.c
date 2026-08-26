// Copyright <2023> <Andrés Serrano>
#include "zippass.h"
#include <assert.h>
#include <limits.h>
#include <math.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "array_strings.h"
#include "misc.h"
#include "copies.h"
#include "queue.h"
#include "threads.h"
#include "zippass_threads.h"


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
  // Queue used by the producer to enqueue generated passwords and by the
  // consumers to dequeue them and brute force
  pthread_mutex_t can_access_status;
  // Mutex to let consumers open their zip (and avoid a tsan warning)
  pthread_mutex_t can_open_zip;
  // Boolean that states whether the zip file has already been opened or not.
  // This boolean is checked by the consumers and it is protected by the mutex
  // 'can_access_status'.
  sem_t can_dequeue_password;

  queue_t passwords_queue;

  bool brute_forced;
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
      if (pthread_mutex_init(&zippass->can_access_status, NULL)
          == EXIT_SUCCESS) {
        if (queue_init(&zippass->passwords_queue) == EXIT_SUCCESS) {
          if (sem_init(&zippass->can_dequeue_password, 0, 0) == EXIT_SUCCESS) {
            // Initialize variables
            zippass->passwords_max_length = 0;
            zippass->thread_count = sysconf(_SC_NPROCESSORS_ONLN);
            pthread_mutex_init(&zippass->can_open_zip, NULL);
          }
        }
      } else {
      free(zippass->alphabet);
      zippass_destroy(zippass);
      }
      // Initialize variables
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

// [ ] Change this so it deletes only if it has been allocated
void zippass_destroy(zippass_t* zippass) {
  pthread_mutex_destroy(&zippass->can_open_zip);
  pthread_mutex_destroy(&zippass->can_access_status);
  queue_destroy(&zippass->passwords_queue);
  sem_destroy(&zippass->can_dequeue_password);
  free(zippass->alphabet);
  free(zippass);
}

int zippass_brute_force(zippass_t* zippass, char* zip_path, char* pass_ret) {
  zippass->brute_forced = false;
  // Create 'thread_count' copies for each consumer thread
  array_strings_t* zip_copies = create_copies(zip_path,
      zippass->thread_count);

  if (zip_copies) {
    // Allocate data for each consumer
    zippass_thread_data_t* consumers_data =
        zippass_create_threads_data(zippass, zip_copies, pass_ret);

    if (consumers_data) {
      // Start consumer threads
      pthread_t *consumers = zippass_create_threads(consumers_data,
          zippass->thread_count, zippass_consume_passwords);

      // Begin password generation
      // printf("Main thread: Attacking %s\n", zip_path);
      zippass_produce_passwords(zippass);

      // If none of the passwords will work, let consumers find the queue empty
      // and break from their while
        for (uint64_t i = 0; i < zippass->thread_count; ++i) {
          password_block_t block;
          block.password_length = -1;
          queue_enqueue(&zippass->passwords_queue, block);
          sem_post(&zippass->can_dequeue_password);
        }

      // Join consumers
      zippass_join_threads(zippass->thread_count, consumers);
      // printf("Main thread: Done attacking %s\n", zip_path);
      // puts("");
      // Destroy data
      zippass_destroy_threads_data(consumers_data, zippass->thread_count);

      // Empty queue and "reset" semaphore
      uint64_t count = zippass->passwords_queue.count;
      for (uint64_t i = 0; i < count; ++i) {
        sem_wait(&zippass->can_dequeue_password);
        queue_dequeue_no_return(&zippass->passwords_queue);
      }
    } else {
    }
    remove_files(zip_copies);
  } else {
    fprintf(stderr, "Error: Couldn't create copies\n");
  }

  // Return empty string if brute force failed
  if (!zippass->brute_forced) {
    snprintf(pass_ret, strlen("")+1, "%s", "");
  }

  return 0;
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

// Producer procedure
// [ ] Document change
void zippass_produce_passwords(zippass_t* zippass) {
  uint64_t max_length = zippass->passwords_max_length;
  uint64_t alphabet_length = (uint64_t) strlen(zippass->alphabet);

  for (uint64_t i = 0; i< max_length; ++i) {
    uint64_t current_password_length = i+1;
    uint64_t count = (uint64_t) pow(alphabet_length, current_password_length);
    password_block_t block;

    if (count > zippass->thread_count) {
      #if 1
      for (uint64_t j = 0; j < zippass->thread_count; ++j) {
        block.start = map(j, count, zippass->thread_count);
        block.password_length = current_password_length;
        block.finish = map(j+1, count, zippass->thread_count) - 1;
        if (j == zippass->thread_count-1) {
          ++block.finish;
        }
        queue_enqueue(&zippass->passwords_queue, block);
        sem_post(&zippass->can_dequeue_password);
      }
      #endif
    } else {
      block.start = 0;
      block.finish = count;
      block.password_length = current_password_length;
      queue_enqueue(&zippass->passwords_queue, block);
      sem_post(&zippass->can_dequeue_password);
    }
  }
}

// Consumer procedure
void* zippass_consume_passwords(void* data) {
  // Declare private and shared data
  zippass_thread_data_t *private_data = (zippass_thread_data_t*) data;
  zippass_t *shared_data = private_data->zippass;

  // Buffer to store passwords
  char my_password[100] = {0};
  // Boolean to check if the password has been found yet
  bool password_found = false;
  // Open zip
  pthread_mutex_lock(&shared_data->can_open_zip);
  zip_t *my_zip_copy = zip_open(private_data->my_zip_path_copy, 0, NULL);
  pthread_mutex_unlock(&shared_data->can_open_zip);

  // Get the alphabet length for convenience
  uint64_t alphabet_length = strlen(shared_data->alphabet);
  if (my_zip_copy) {
    while (!password_found) {
      password_block_t my_block;
      sem_wait(&shared_data->can_dequeue_password);
      queue_dequeue(&shared_data->passwords_queue, &my_block);

      if (my_block.password_length == -1) {
        break;
      }

      for (uint64_t i = my_block.start; i < my_block.finish && !password_found;
          ++i) {
        generate_string(i, my_block.password_length, shared_data->alphabet,
            alphabet_length, my_password);
        if (zippass_fopen(my_zip_copy, my_password) == ZIP_FILE_OPENED) {
            snprintf(private_data->password_return, strlen(my_password)+1, "%s",
                my_password);
            pthread_mutex_lock(&shared_data->can_access_status);
            shared_data->brute_forced = true;
            pthread_mutex_unlock(&shared_data->can_access_status);
        }

        pthread_mutex_lock(&shared_data->can_access_status);
        password_found = shared_data->brute_forced;
        pthread_mutex_unlock(&shared_data->can_access_status);
      }
    }
    zip_close(my_zip_copy);
  }
  // printf("\tThread %"PRIu64": Returning\n", private_data->my_thread_number);
  return NULL;
}

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
