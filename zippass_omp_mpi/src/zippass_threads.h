// Copyright [2023] <Andrés Serrano>

#ifndef ZIPPASS_THREADS_H
#define ZIPPASS_THREADS_H

#include <pthread.h>
#include <inttypes.h>
#include "zippass.h"
#include "array_strings.h"

/**
 * @brief Data for each thread created by zippass
 * @details Contains the thread's number, a copy of a zip archive, a pointer to
 * a zippass_t datatype (shared data), and a pointer to a char*
 * (a string of characters) to which the password will be written to.
*/
typedef struct zip_passwords_thread_data {
  uint64_t my_thread_number;
  char* my_zip_path_copy;
  char* password_return;
  zippass_t* zippass;  // <-- shared_data
}zippass_thread_data_t;

/**
 * @brief Create an array of threads and make them run 'subroutine' with
 * paremeter 'threads_data'
 * @details 'threads_data' is an array and each thread is sent with one slot
 * of the array.
 * @param threads_data Data for each thread. This is the parameter for
 * 'subroutine'.
 * @param count Number of threads to create.
 * @param subroutine The subroutine all threads will execute.
 * @returns The array of threads.
*/
pthread_t* zippass_create_threads(zippass_thread_data_t* threads_data,
    uint64_t count, void*(*subroutine)(void*));

/**
 * @brief Join all threads in 'threads'.
 * @details This subroutine is just a for loop that joins all threads from the
 * parameter 'threads' of size 'count'.
 * @param count The number of threads to join.
 * @param threads The array of threads to join.
 * @returns An error code
*/
int zippass_join_threads(size_t count, pthread_t* threads);

/**
 * @brief Create an array of data for a team of threads.
 * @details Allocate an array of zippass_thread_data_t and initialize it with
 * data from 'copies' and share password_return.
 * @param zippass Shared data for the threads. 'zippass' also contains the
 * thread_count.
 * @param copies An array of zip copies. It must the same count as the
 * attribute zippass->thread_count
 * @param password_return A pointer to a string shared for all threads.
 * @returns An array of zippass_thread_data_t if successfully allocated and
 * initialized, or null if an error ocurred.
*/
zippass_thread_data_t* zippass_create_threads_data(zippass_t* zippass,
    array_strings_t* copies, char* password_return);

/**
 * @brief Destroy the array of data for threads.
 * @details Free the string atribute from each element in the array
 * 'threads_data' and then 'threads_data' itself.
 * @param threads_data The array of data that will be destroyed.
 * @param count The size of the array.
*/
void zippass_destroy_threads_data(zippass_thread_data_t* threads_data,
    uint64_t count);

#endif
