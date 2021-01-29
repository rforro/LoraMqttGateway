#include <StringRingBuffer.h>
#include <unity.h>

#define SIZE 100

StringRingBuffer strring(SIZE);

void test_string_push_too_long(void) {
    char text[SIZE+1];
    for (int i=0; i<SIZE; i++) {
        text[i] = (char) random(32, 127);
    }
    text[SIZE] = '\0';

    TEST_ASSERT_EQUAL(-1, strring.pushstr(text));    
}

void test_string_push_max_size(void) {
    char text[SIZE+1];
    for (int i=0; i<SIZE; i++) {
        text[i] = (char) random(32, 127);
    }
    text[SIZE-1] = '\0';

    TEST_ASSERT_EQUAL(SIZE-1, strlen(text));
    TEST_ASSERT_EQUAL(0, strring.pushstr(text));
}

void test_simple_read_write(void) {
    char text[] = "lorem ipsum";
    char target[20];

    strring.reset();

    TEST_ASSERT_NOT_EQUAL(-1,strring.pushstr(text));
    strring.popstr(target);
    TEST_ASSERT_EQUAL_STRING(text, target);
}

void test_extensive_read_write(void) {
    char text[] = "lorem ipsum";
    char target[20];
    size_t len = strlen(text);

    strring.reset();

    // fill buffer, try insert one more
    for (size_t i=len; i<SIZE; i=i+len+1) {
        TEST_ASSERT_NOT_EQUAL(-1, strring.pushstr(text));
    }
    TEST_ASSERT_EQUAL(-1, strring.pushstr(text));

    // read one, write one
    strring.popstr(target);
    TEST_ASSERT_NOT_EQUAL(-1, strring.pushstr(text));

    // read all out
    while(!strring.is_empty()) {
        strring.popstr(target);
    }
    TEST_ASSERT_EQUAL(SIZE-1, strring.get_left_space());
    TEST_ASSERT_NOT_EQUAL(-1, strring.pushstr(text));
}

void process() {
    UNITY_BEGIN();
    RUN_TEST(test_string_push_too_long);
    RUN_TEST(test_string_push_max_size);
    RUN_TEST(test_simple_read_write);
    RUN_TEST(test_extensive_read_write);
    UNITY_END();
}

#ifdef ARDUINO

#include <Arduino.h>
void setup() {
    delay(2000);
    randomSeed(analogRead(A0));
    process();
}

void loop() {}

#else

int main(int argc, char **argv) {
    process();
    return 0;
}

#endif