#ifndef _PKCS7_PADDING_H_
#define _PKCS7_PADDING_H_

#include <stdint.h>
#include <stddef.h>

/**
 * Pad a buffer with bytes as defined in PKCS#7 
 * 
 * @return number of pad bytes added or zero and less if
 * the buffer size is not large enough to hold the correctly padded data
 */
int pkcs7_padding_pad_buffer(char *buf, size_t data_length, size_t buf_size, uint8_t modulus);

/**
 * Checks if padding is valid or not
 * @param buffer char array with padding
 * @param data_length length of unpadded string
 * @param buffer_size total size of char array
 * @param modulus block size
 * @return 1 on succes, 0 on error
 */
int pkcs7_padding_valid(char *buf, size_t data_len, size_t buf_s, uint8_t modulus);

/**
 * Given a block of pkcs7 padded data, return the actual data length without padding.
 * @param buf string in cleartext with padding
 * @param str_length length of string with padding
 * @param modulus block size 
 * @return length of valid data, 0 on error
 */
size_t pkcs7_padding_data_length(unsigned char *buf, size_t data_len, uint8_t modulus);

#endif