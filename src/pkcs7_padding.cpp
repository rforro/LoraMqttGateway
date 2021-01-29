#include "pkcs7_padding.h"

int pkcs7_padding_pad_buffer(char *buf, size_t data_len, size_t buf_size, uint8_t modulus)
{
    uint8_t pad_byte = modulus - (data_len % modulus);
    if (data_len + pad_byte >= buf_size)
    {
        return -pad_byte;
    }
    int i = 0;
    while (i < pad_byte)
    {
        buf[data_len + i] = pad_byte;
        i++;
    }
    buf[data_len + i] = '\0';
    return pad_byte;
}

int pkcs7_padding_valid(char *buf, size_t data_len, size_t buf_s, uint8_t modulus)
{
    uint8_t expected_pad_byte = modulus - (data_len % modulus);
    if (data_len + expected_pad_byte >= buf_s)
    {
        return 0;
    }
    int i = 0;
    while (i < expected_pad_byte)
    {
        if (buf[data_len + i] != expected_pad_byte)
        {
            return 0;
        }
        i++;
    }
    return 1;
}

size_t pkcs7_padding_data_length(unsigned char *buf, size_t data_len, uint8_t modulus)
{
    /* test for valid buffer size */
    if (data_len % modulus != 0 ||
        data_len < modulus)
    {
        return 0;
    }
    uint8_t padding_value;
    padding_value = buf[data_len - 1];
    /* test for valid padding value */
    if (padding_value < 1 || padding_value > modulus)
    {
        return 0;
    }
    /* buffer must be at least padding_value + 1 in size */
    if (data_len < padding_value + 1)
    {
        return 0;
    }
    uint8_t count = 1;
    data_len--;
    for (; count < padding_value; count++)
    {
        data_len--;
        if (buf[data_len] != padding_value)
        {
            return 0;
        }
    }
    return data_len;
}