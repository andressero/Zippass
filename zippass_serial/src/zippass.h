// Copyright <2023> <Andrés Serrano>
#ifndef ZIPPASS_H
#define ZIPPASS_H

#include <stdint.h>
#include <zip.h>

#define BRUTE_FORCED 30
#define NOT_BRUTE_FORCED 31
#define ZIP_NOT_OPENED 32

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
*/
void zippass_destroy(zippass_t* zippass);


/**
 * @brief Set the password's alphabet.
*/
void zippass_set_alphabet(zippass_t* zippass, char* alphabet);

/**
 * @brief Set maximum length of password.
*/
void zippass_set_passwords_max_length(zippass_t* zippass, uint64_t length);

/**
 * @brief Get the password's alphabet.
*/
void zippass_get_alphabet(zippass_t* zippass, char* retval);

/**
 * @brief Get the maximum length allowed for a password.
 * @returns The maximum length allowed for a password.
*/
uint64_t zippass_get_passwords_max_length(zippass_t* zippass);


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
 * @returns A status indicating whether the zip could be opened and/or brute
 * forced. 30 means the brute force attack was successful.
*/
int zippass_brute_force(zippass_t* zippass, char* path_to_zip,
  char* password_return);

# endif
