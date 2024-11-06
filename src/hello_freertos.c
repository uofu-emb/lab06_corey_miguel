/**
 * Copyright (c) 2022 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <FreeRTOS.h>
#include <semphr.h>

#include "task.h"
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/cyw43_arch.h"

int count = 0;
bool on = false;
SemaphoreHandle_t semaphore;

#define MAIN_TASK_PRIORITY (tskIDLE_PRIORITY + 4UL)
#define HIGH_PRIORITY_TASK_PRIORITY (tskIDLE_PRIORITY + 3UL)
#define MEDIUM_PRIORITY_TASK_PRIORITY (tskIDLE_PRIORITY + 2UL)
#define LOW_PRIORITY_TASK_PRIORITY (tskIDLE_PRIORITY + 1UL)
#define MAIN_TASK_STACK_SIZE configMINIMAL_STACK_SIZE
#define PRIORITY_TASK_STACK_SIZE configMINIMAL_STACK_SIZE

void high_priority_task(__unused void *params)
{
  /* See if we can obtain the semaphore. If the semaphore is not
      available wait 10 ticks to see if it becomes free.
  */
  printf("High priority task attempting to take the semaphore.\n");

  if (xSemaphoreTake(semaphore, (TickType_t)10000) == pdTRUE)
  {
    printf("High priority task was able to obtain the semaphore.\n");

    xSemaphoreGive(semaphore);
    printf("High priority work done and semaphore was released.\n");
  }
  else
  {
    printf("High priority unable to take semaphore.\n");
  }

  while (1)
  {
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
}

void medium_priority_task(__unused void *params)
{
  printf("Medium priority task attempting to starve system.\n");
  while (1)
  {
  }
}

void low_priority_task(__unused void *params)
{
  /* See if we can obtain the semaphore. If the semaphore is not
      available wait 10 ticks to see if it becomes free.
  */
  printf("Low priority task attempting to take the semaphore.\n");

  if (xSemaphoreTake(semaphore, (TickType_t)10) == pdTRUE)
  {
    printf("Low priority task was able to obtain the semaphore.\n");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    xSemaphoreGive(semaphore);
    printf("Low priority work done and semaphore was released.\n");
  }
  else
  {
    /* We could not obtain the semaphore and can therefore not access
        the shared resource safely. */
    printf("Low priority unable to take semaphore.\n");
  }

  while (1)
  {
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
}

void supervisor_task(__unused void *params)
{
  // Create low priority task
  xTaskCreate(low_priority_task, "low_priority_thread",
              PRIORITY_TASK_STACK_SIZE, NULL, LOW_PRIORITY_TASK_PRIORITY, NULL);  

  vTaskDelay(500 / portTICK_PERIOD_MS);

  xTaskCreate(medium_priority_task, "medium_priority_thread",
              PRIORITY_TASK_STACK_SIZE, NULL, MEDIUM_PRIORITY_TASK_PRIORITY, NULL);
  
  // Create high priority task
  xTaskCreate(high_priority_task, "high_priority_thread",
              PRIORITY_TASK_STACK_SIZE, NULL, HIGH_PRIORITY_TASK_PRIORITY, NULL);

  while (1)
  {
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
}

int main(void)
{
  stdio_init_all();
  sleep_ms(5000); // Give time for TTY to attach.

  semaphore = xSemaphoreCreateMutex();
  
  if (!semaphore)
  {
    printf("Error creating semaphore. %s\n.", semaphore);
    return 0;
  }

  printf("Semaphore was created successfully.\n");
  const char *rtos_name;
  rtos_name = "FreeRTOS";
  TaskHandle_t task;
  xTaskCreate(supervisor_task, "Supervisor_thread",
              MAIN_TASK_STACK_SIZE, NULL, MAIN_TASK_PRIORITY, &task);

  vTaskStartScheduler();
  return 0;
}
