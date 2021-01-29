#include "StringRingBuffer.h"

StringRingBuffer::StringRingBuffer(size_t len)
{
    this->len = len;
    this->buffer = new char[len];
    this->ri = this->wri = 0;
}

StringRingBuffer::~StringRingBuffer()
{
    delete this->buffer;
}

/**
 * Inserts n characters into string ring buffer
 * @param s pointer to char array
 * @param len number of characters to be inserted
 * @return on success 0, on error -1.
 */
int StringRingBuffer::pushnstr(char *s, size_t len)
{
    if (len > get_left_space())
        return -1;

    char *p = s;

    for (int i = 0; i < len; i++)
    {
        this->buffer[this->wri] = *(p + i);
        _add_wri();
    }

    this->buffer[this->wri] = STRING_SEPPARATOR;
    _add_wri();

    return 0;
}

/**
 * Inserts NULL-terminated string into ring buffer.
 * @param s pointer to NULL-terminated char array
 * @return on success 0, on error -1.
 */
int StringRingBuffer::pushstr(char *s)
{
    return pushnstr(s, strlen(s));
}

/**
 * Copies string from ring buffer into target.
 * 
 * Caution this function cannot check if target array
 * is long enough. It has to be at least n+1 chars long.
 * 
 * @return on success number of chars written, on error -1.
 */
int StringRingBuffer::popstr(char *target)
{
    if (is_empty())
        return -1;

    size_t bi = this->ri;
    char *p = target;

    while (buffer[this->ri] != STRING_SEPPARATOR)
    {
        *p++ = this->buffer[this->ri];
        _add_ri();
    }
    *p = '\0';

    _add_ri(); //step over string sepparator

    return this->ri - bi - 1;
}

/**
 * Calculates free space in buffer
 * @return number of available chars
 */
size_t StringRingBuffer::get_left_space()
{
    if (this->wri < this->ri)
    {
        return this->ri - this->wri - 1;
    }

    int left = this->len - this->wri + this->ri;

    return left < 1 ? 0 : left - 1;
}

/**
 * Checks if ring buffer is empty
 * @return true if empty, otherwise false
 */
bool StringRingBuffer::is_empty()
{
    return (this->wri == this->ri);
}

/**
 * Resets ring buffer content
 * This basically deletes its content
 */
void StringRingBuffer::reset()
{
    this->wri = this->ri = 0;
}

void StringRingBuffer::_add_wri(void)
{
    this->wri = (this->wri + 1) % this->len;
}

void StringRingBuffer::_add_ri(void)
{
    this->ri = (this->ri + 1) % this->len;
}