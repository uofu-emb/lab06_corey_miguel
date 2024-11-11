#include <stdio.h>
#include <pico/stdlib.h>
#include <stdint.h>
#include <unity.h>
#include "unity_config.h"

#include <FreeRTOS.h>
#include "task.h"

#include "busy.h"

#define LOW_PRIORITY ( tskIDLE_PRIORITY + 1UL )
#define HIGH_PRIORITY ( tskIDLE_PRIORITY + 2UL )
#define SUPERVISOR_PRIORITY ( tskIDLE_PRIORITY + 3UL )

void setUp(void) {}

void tearDown(void) {}

void test1Supervisor(__unused void *params) {
    TaskHandle_t t1, t2;

    xTaskCreate(busy_busy, "t1", configMINIMAL_STACK_SIZE, NULL, LOW_PRIORITY, &t1);
    xTaskCreate(busy_yield, "t2", configMINIMAL_STACK_SIZE, NULL, LOW_PRIORITY, &t2);

    vTaskDelay(10000 / portTICK_PERIOD_MS);

    vTaskSuspend(t1);
    vTaskSuspend(t2);

    TaskStatus_t t1_status, t2_status;

    vTaskGetInfo(t1, &t1_status, pdTRUE, eInvalid);
    vTaskGetInfo(t2, &t2_status, pdTRUE, eInvalid);

    
    printf("Task 1 Runtime: %ld\n", t1_status.ulRunTimeCounter);
    printf("Task 2 Runtime: %ld\n", t2_status.ulRunTimeCounter);

    while(1) {}
}

void test1(void) {    
    xTaskCreate(test1Supervisor, "sup", configMINIMAL_STACK_SIZE, NULL, SUPERVISOR_PRIORITY, NULL);
    vTaskStartScheduler();    
}

int main (void)
{
    stdio_init_all();
    sleep_ms(5000); // Give time for TTY to attach.
    printf("Start tests\n");
    UNITY_BEGIN();
    RUN_TEST(test1);
    sleep_ms(5000);
    return UNITY_END();
}
