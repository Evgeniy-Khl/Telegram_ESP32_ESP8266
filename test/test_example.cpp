#include <Arduino.h>
#include <unity.h>

void test_example() {
    TEST_ASSERT_EQUAL(1, 1); // Простой тест
}

void setup() {
    UNITY_BEGIN();
    RUN_TEST(test_example);
    UNITY_END();
}

void loop() {
    // Нет необходимости в реализации loop для тестов
}