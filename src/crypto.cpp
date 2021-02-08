#include "crypto.h"
#include "mbedtls/aes.h"
#include "pkcs7_padding.h"
#include "gateway.h"

#define BLOCK_SIZE 16

const unsigned char key[BLOCK_SIZE] = KEY;
const unsigned char iv_key[BLOCK_SIZE] = IV_KEY;

mbedtls_aes_context aes;

int decrypt_packet(char *plain, size_t plain_len, char *encrypted, size_t encrypted_len)
{
    unsigned char iv_recv[BLOCK_SIZE] = {0};
    unsigned char iv_static[BLOCK_SIZE] = IV_STATIC;
    unsigned char iv[BLOCK_SIZE] = {0};
    unsigned char msg_crypted[encrypted_len];
    unsigned char msg_padded[encrypted_len];

    // decrypt IV for payload
    memcpy(iv_recv, encrypted, BLOCK_SIZE);
    mbedtls_aes_init(&aes);
    if (mbedtls_aes_setkey_dec(&aes, iv_key, BLOCK_SIZE*8) != 0) {
        return SETKEY_ERROR;
    }
    if (mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_DECRYPT, BLOCK_SIZE, iv_static, (const unsigned char*) iv_recv, iv) != 0) {
        return DECRYPT_IV_ERROR;
    }
    mbedtls_aes_free(&aes);

    // decrypt payload with decrypted IV
    size_t msg_len = encrypted_len - sizeof(iv_recv);
    memcpy(msg_crypted, &encrypted[sizeof(iv_recv)], msg_len);
    mbedtls_aes_init(&aes);
    if (mbedtls_aes_setkey_dec(&aes, key, BLOCK_SIZE*8) != 0) {
        return SETKEY_ERROR;
    }
    if (mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_DECRYPT, msg_len, iv, (const unsigned char*) msg_crypted, msg_padded) != 0) {
        return DECRYPT_MSG_ERROR;
    }
    mbedtls_aes_free(&aes);

    size_t msg_unpadded_len = pkcs7_padding_data_length(msg_padded, msg_len, BLOCK_SIZE);
    if (msg_unpadded_len == 0) {
        return PADDING_ERROR;
    }

    if (strlcpy(plain, (char*)msg_padded, msg_unpadded_len + 1) >= plain_len) { // msg_padded is not NULL terminated
        return PLAIN_LENGTH_ERROR;
    }

    return 0;
}