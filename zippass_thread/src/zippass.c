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
  queue_t passwords_queue;
  // Semaphore to prevent consumers from "dequeueing" from an empty queue
  sem_t can_dequeue_password;
  // Mutex to let consumers check the current status of the brute force attack
  pthread_mutex_t can_access_status;
  // Mutex to let consumers open their zip (and avoid a tsan warning)
  pthread_mutex_t can_open_zip;
  // Boolean that states whether the zip file has already been opened or not.
  // This boolean is checked by the consumers and it is protected by the mutex
  // 'can_access_status'.
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
 * @brief Generate a password of length 'password_length' using 'alphabet' and
 * 'index'. Return password in 'password_return'.
 * @details This routine creates a password, character by character, in a for
 * loop that starts at index 0 and ends at 'password_length'. To get characters
 * from the alphabet, first it calculates the length of the alphabet and stores
 * it in a variable called 'base'. Then, it calculates indexes operating
 * 'index' modulo 'base' and then dividing 'index' by 'base'.
 * @param index An index to generate the password number 'index'.
 * @param password_length Desired length of the password
 * @param password_return Pointer to write the password to.
 * @param alphabet An alphabet to generate a password from.
 * @see zippass_produce_passwords
*/
void zippass_generate_password(uint64_t index, uint64_t password_length,
    char* password_return, char* alphabet);

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
      // Initialize queue
      if (queue_init(&zippass->passwords_queue) == EXIT_SUCCESS) {
        // Initialize consumer semaphore
        if (sem_init(&zippass->can_dequeue_password, 0, 0) == EXIT_SUCCESS) {
          if (pthread_mutex_init(&zippass->can_access_status, NULL)
              == EXIT_SUCCESS) {
            // Initialize variables
            zippass->passwords_max_length = 0;
            zippass->thread_count = sysconf(_SC_NPROCESSORS_ONLN);
            pthread_mutex_init(&zippass->can_open_zip, NULL);
          } else {
          sem_destroy(&zippass->can_dequeue_password);
          queue_destroy(&zippass->passwords_queue);
          free(zippass->alphabet);
          zippass_destroy(zippass);
          }
        } else {
          fprintf(stderr, "Error: Couldn't initialize consumer's "
              "semaphore\n");
          queue_destroy(&zippass->passwords_queue);
          free(zippass->alphabet);
          zippass_destroy(zippass);
        }
      } else {
        free(zippass->alphabet);
        zippass_destroy(zippass);
        fprintf(stderr, "Error: Couldn't initialize passwords queue\n");
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

void zippass_destroy(zippass_t* zippass) {
  pthread_mutex_destroy(&zippass->can_open_zip);
  pthread_mutex_destroy(&zippass->can_access_status);
  sem_destroy(&zippass->can_dequeue_password);
  queue_destroy(&zippass->passwords_queue);
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
      zippass_produce_passwords(zippass);

      // If none of the passwords will work, let consumers find the queue empty
      // and break from their while
        for (uint64_t i = 0; i < zippass->thread_count; ++i) {
          queue_enqueue(&zippass->passwords_queue, " ");
          sem_post(&zippass->can_dequeue_password);
        }

      // Join consumers
      zippass_join_threads(zippass->thread_count, consumers);

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
    remove_copies(zip_copies);
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
void zippass_produce_passwords(zippass_t* zippass) {
  char password[100] = {0};
  uint64_t max_length = zippass->passwords_max_length;
  uint64_t alphabet_length = (uint64_t) strlen(zippass->alphabet);

  for (uint64_t i = 0; i< max_length; ++i) {
    uint64_t current_password_length = i+1;
    uint64_t count = (uint64_t) pow(alphabet_length, current_password_length);

    for (uint64_t j = 0; j < count; ++j) {
      zippass_generate_password(j, current_password_length,
          password, zippass->alphabet);
      queue_enqueue(&zippass->passwords_queue, password);
      sem_post(&zippass->can_dequeue_password);
    }
  }
}

// Consumer procedure
void* zippass_consume_passwords(void* data) {
  zippass_thread_data_t *private_data = (zippass_thread_data_t*) data;
  zippass_t *shared_data = private_data->zippass;
  char my_password[100] = {0};
  pthread_mutex_lock(&shared_data->can_open_zip);
  zip_t *my_zip_copy = zip_open(private_data->my_zip_path_copy, 0, NULL);
  pthread_mutex_unlock(&shared_data->can_open_zip);
  bool password_found = false;

  if (my_zip_copy) {
    while (true) {
      // First check if the password has already been found
      pthread_mutex_lock(&shared_data->can_access_status);
      password_found = shared_data->brute_forced;
      pthread_mutex_unlock(&shared_data->can_access_status);
      if (password_found) {
        // Another thread found the password
        break;
      }
      // Get a password
      sem_wait(&shared_data->can_dequeue_password);
      queue_dequeue(&shared_data->passwords_queue, my_password);
      // Check if its the stop condition
      if (strcmp(my_password, " ") != 0) {
        // Try to open the file using the password
        if (zippass_fopen(my_zip_copy, my_password) == ZIP_FILE_OPENED) {
          // Write password to return
          snprintf(private_data->password_return, strlen(my_password)+1, "%s",
              my_password);
          pthread_mutex_lock(&shared_data->can_access_status);
          shared_data->brute_forced = true;
          pthread_mutex_unlock(&shared_data->can_access_status);
          // This thread found the password
          break;
        }
      } else {
        // This thread found the stop condition
        break;
      }
    }
    zip_close(my_zip_copy);
  } else {
  }
  return NULL;
}

void zippass_generate_password(uint64_t index, uint64_t password_length,
  char* password_return, char* alphabet) {
  uint64_t modulo = 0;
  uint64_t i = 0;
  // Numeric base is the length of the alphabet
  uint64_t base = strlen(alphabet);

  for (i = 0; i < password_length; ++i) {
    // Get a valid index (for alphabet) using modulo
    modulo = index % base;
    // Write in password_return the character from alphabet at index 'modulo'
    password_return[i] = alphabet[modulo];
    // Divide index by base to get it ready for next modulo operation
    index = index / base;
  }

  password_return[i] = '\0';
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
