#include <StringRingBuffer.h>
#include <unity.h>

#define SIZE 100

StringRingBuffer strring(SIZE);

void test_is_empty(void) {
    TEST_ASSERT_TRUE(strring.is_empty());
}

void test_is_not_empty(void) {
    char text[] = "loremipsum";
    strring.pushstr(text);
    TEST_ASSERT_FALSE(strring.is_empty());
}

void test_reset(void) {
    char text[] = "lorem ipsum";
    strring.pushstr(text);
    strring.reset();
    TEST_ASSERT_TRUE(strring.is_empty());
}

void test_is_emtpy_after_rw(void) {
    char write[] = "lorem ipsum";
    char read [20];

    strring.pushstr(write);
    strring.popstr(read);
    TEST_ASSERT_TRUE(strring.is_empty());
}

void process() {
    UNITY_BEGIN();
    RUN_TEST(test_is_empty);
    RUN_TEST(test_is_not_empty);
    RUN_TEST(test_reset);
    RUN_TEST(test_is_emtpy_after_rw);
    UNITY_END();
}

#ifdef ARDUINO

#include <Arduino.h>
void setup() {
    delay(2000);
    process();
}

void loop() {}

#else

int main(int argc, char **argv) {
    process();
    return 0;
}

#endif