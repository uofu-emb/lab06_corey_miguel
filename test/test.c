#include <stdio.h>
#include <pico/stdlib.h>
#include <stdint.h>
#include <unity.h>
#include "unity_config.h"

#include <FreeRTOS.h>
#include "task.h"
#include <pico/time.h>

#include "busy.h"

#define LOW_PRIORITY ( tskIDLE_PRIORITY + 1UL )
#define HIGH_PRIORITY ( tskIDLE_PRIORITY + 2UL )
#define SUPERVISOR_PRIORITY ( tskIDLE_PRIORITY + 3UL )

void setUp(void) {}

void tearDown(void) {}

void test(TaskFunction_t t1_func, TaskFunction_t t2_func, uint64_t *t1_time, uint64_t *t2_time, uint64_t t1_t2_delay_ms, BaseType_t t1_priority, BaseType_t t2_priority) {
    TaskHandle_t t1, t2;

    TickType_t start_ticks = xTaskGetTickCount();
    uint64_t start_count = portGET_RUN_TIME_COUNTER_VALUE();

    xTaskCreate(t1_func, "t1", configMINIMAL_STACK_SIZE, NULL, t1_priority, &t1);

    vTaskDelay(t1_t2_delay_ms / portTICK_PERIOD_MS);

    xTaskCreate(t2_func, "t2", configMINIMAL_STACK_SIZE, NULL, t2_priority, &t2);

    vTaskDelay(1000 / portTICK_PERIOD_MS);

    TaskStatus_t t1_status, t2_status;

    vTaskGetInfo(t1, &t1_status, pdTRUE, eInvalid);
    vTaskGetInfo(t2, &t2_status, pdTRUE, eInvalid);

    *t1_time = ulTaskGetRunTimeCounter(t1);
    *t2_time = ulTaskGetRunTimeCounter(t2);
    
    printf("Task 1 Runtime: %llu\n", *t1_time);
    printf("Task 2 Runtime: %llu\n", *t2_time);

    vTaskDelete(t1);
    vTaskDelete(t2);
}

void both_busy_busy(void) {
    uint64_t t1_time = 0, t2_time = 0;
    test(busy_busy, busy_busy, &t1_time, &t2_time, 0, LOW_PRIORITY, LOW_PRIORITY);

    TEST_ASSERT(t1_time > 400000 && t1_time < 600000);
    TEST_ASSERT(t2_time > 400000 && t2_time < 600000);
}

void both_busy_yield(void) {
    uint64_t t1_time = 0, t2_time = 0;
    test(busy_yield, busy_yield, &t1_time, &t2_time, 0, LOW_PRIORITY, LOW_PRIORITY);

    TEST_ASSERT(t1_time > 400000 && t1_time < 600000);
    TEST_ASSERT(t2_time > 400000 && t2_time < 600000);
}

void t1_busy_t2_yield(void) {
    uint64_t t1_time = 0, t2_time = 0;
    test(busy_busy, busy_yield, &t1_time, &t2_time, 0, LOW_PRIORITY, LOW_PRIORITY);

    TEST_ASSERT(t1_time > 900000);
    TEST_ASSERT(t2_time < 100000);
}

void high_priority_first(void) {
    uint64_t t1_time = 0, t2_time = 0;
    test(busy_busy, busy_busy, &t1_time, &t2_time, 100, HIGH_PRIORITY, LOW_PRIORITY);

    TEST_ASSERT(t1_time > 900000);
    TEST_ASSERT(t2_time == 0);
}

void low_priority_first(void) {
    uint64_t t1_time = 0, t2_time = 0;
    test(busy_busy, busy_busy, &t1_time, &t2_time, 100, LOW_PRIORITY, HIGH_PRIORITY);

    TEST_ASSERT(t1_time < 100000);
    TEST_ASSERT(t2_time > 900000);
}

void busy_yield_different_priorities(void) {
    uint64_t t1_time = 0, t2_time = 0;
    test(busy_yield, busy_yield, &t1_time, &t2_time, 0, HIGH_PRIORITY, LOW_PRIORITY);

    TEST_ASSERT(t1_time > 900000);
    TEST_ASSERT(t2_time == 0);
}

void test_supervisor(__unused void *params) {   
    while(1) {
        sleep_ms(5000); // Give time for TTY to attach.
        printf("Start tests\n");
        UNITY_BEGIN(); 
        RUN_TEST(both_busy_busy);
        RUN_TEST(both_busy_yield);
        RUN_TEST(t1_busy_t2_yield);
        RUN_TEST(high_priority_first);
        RUN_TEST(low_priority_first);
        RUN_TEST(busy_yield_different_priorities);
        UNITY_END();
    }
}

int main (void)
{
    stdio_init_all();
    
    xTaskCreate(test_supervisor, "sup", configMINIMAL_STACK_SIZE, NULL, SUPERVISOR_PRIORITY, NULL);
    vTaskStartScheduler();
    
    return 0;
}
