#ifndef STRINGRINGBUFFER_H
#define STRINGRINGBUFFER_H

#include <Arduino.h>

#ifndef STRRINGBUFFER_DEF_SIZE
#define STRRINGBUFFER_DEF_SIZE 20
#endif

#define STRING_SEPPARATOR 0x3

class StringRingBuffer
{
private:
    char *buffer;
    size_t len, ri, wri;
    void _add_ri(void);
    void _add_wri(void);

public:
    StringRingBuffer(size_t len = STRRINGBUFFER_DEF_SIZE);
    ~StringRingBuffer();

    int pushnstr(char *s, size_t len);
    int pushstr(char *s);
    int popstr(char *target);
    size_t get_left_space();
    bool is_empty();
    void reset();
};

#endif //STRINGRINGBUFFER_H