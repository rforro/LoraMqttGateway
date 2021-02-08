#include <Arduino.h>

#define PADDING_ERROR -1
#define SETKEY_ERROR -2
#define DECRYPT_IV_ERROR -3
#define DECRYPT_MSG_ERROR -4
#define PLAIN_LENGTH_ERROR -5

/**
 * Decrypts AES CBC encrypted message
 * with encrypted IV and paylod also
 * removes PKCS#7 padding.
 * 
 * @param plain char array for decrypted payload
 * @param plain_len lengths of char array for decrypted payload
 * @param encryted char array with encrypted msg
 * @param len length of encrypted msg
 * @return 0 if successful, or error code
 */
int decrypt_packet(char *plain, size_t plain_len, char *encrypted, size_t encrypted_len);