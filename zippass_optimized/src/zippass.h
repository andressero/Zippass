// Copyright <2023> <Andrés Serrano>
#ifndef ZIPPASS_H
#define ZIPPASS_H

#include <stdint.h>
#include <zip.h>

#define BRUTE_FORCED 30
#define NOT_BRUTE_FORCED 31
#define ZIP_NOT_OPENED 32

/**
 * @brief A struct to handle password generation and brute force attacks to zip
 * archives.
 * @details This struct contains all variables necessary to generate passords.
 * The variables 'alphabet' and 'passwords_max_length' are enough for password
 * generation and brute forcing (serially). The other variables are needed do
 * ensure proper concurrency: 
 * - 'thread_count': How many threads will be created
 * - 'passwords_queue': A queue to enqueue and dequeue passwords. It's the main
 *    mean of communication between the producer and the consumers.
 * - 'can_dequeue_password': A semaphore to prevent consumers from
 *    dequeueing from an empty password.
 * - 'can_access_status': A mutex to let threads know the status of the
 *    encrypted zip file (check if it has already been opened or not).
 * - 'can_open_zip': A mutex to let each thread open its zip archive.
 * - 'brute forced': boolean that tells if the zip file has been brute forced or
 *    not. All threads check on this variable regularly. It is protected by the
 *    mutex 'can_access_status'.
*/
typedef struct zip_passwords zippass_t;

/**
 * @brief Constructor for zippass_t datatypes.
 * @details Allocates a zippass_t and initializes its values.
 * @returns A pointer to a zippass_t datatype.
*/
zippass_t* zippass_create();

/**
 * @brief Destructor for zippass_t datatypes.
 * @details Frees all memory allocated by zippass_t and then frees the 
 * zippass_t itself.
 * @param zippass The zippass datatype.
*/
void zippass_destroy(zippass_t* zippass);


/**
 * @brief Set the password's alphabet.
 * @param zippass The zippass datatype.
 * @param alphabet A pointer to the new alphabet.
*/
void zippass_set_alphabet(zippass_t* zippass, char* alphabet);

/**
 * @brief Set maximum length of password.
 * @param zippass The zippass datatype.
 * @param length The new length.
*/
void zippass_set_passwords_max_length(zippass_t* zippass, uint64_t length);

/**
 * @brief Set a new thread count for zippass
 * @param zippass The zippass datatype.
 * @param thread_count The new thread count.
*/
void zippass_set_thread_count(zippass_t* zippass, uint64_t thread_count);

/**
 * @brief Get the password's alphabet.
 * @param zippass The zippass datatype.
 * @param retval A pointer to return the alphabet to.
*/
void zippass_get_alphabet(zippass_t* zippass, char* retval);

/**
 * @brief Get the maximum length allowed for a password.
 * @param zippass The zippass datatype
 * @returns The maximum length allowed for a password.
*/
uint64_t zippass_get_passwords_max_length(zippass_t* zippass);

/**
 * @brief Get the thread count of zippass
 * @param zippass The zippass datatype
 * @returns The current thread count
*/
uint64_t zippass_get_thread_count(zippass_t* zippass);

/**
 * @brief Attempt brute force on the zip archive located in 'path_to_zip'. Copy
 * the password in 'password_return' or an empty string if the password
 * couldn't be found.
 * @details Open the zip archive and attempt opening its first file (assuming
 * it's encrypted) with several passwords. This routine generates passwords
 * using the alphabet contained in 'zippass', starting from passwords of length
 * 1 and finishing with passwords of the maximum length specified; also
 * contained in 'zippass'. If one of the generated passwords successfully opens
 * the encrypted file, that password will be copied into 'password_return'. If
 * none of the passwords opened it, an empty string will be copied instead.
 * @param zippass The zippass datatype
 * @param path_to_zip The path to the zip that will be brute forced
 * @param password_return A pointer to write the password to.
 * @returns A status indicating whether the zip could be opened and/or brute
 * forced. 30 means the brute force attack was successful.
*/
int zippass_brute_force(zippass_t* zippass, char* path_to_zip,
  char* password_return);

# endif
