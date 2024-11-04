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


#define MAIN_TASK_PRIORITY      ( tskIDLE_PRIORITY + 3UL )
#define HIGH_PRIORITY_TASK_PRIORITY     ( tskIDLE_PRIORITY + 2UL )
#define LOW_PRIORITY_TASK_PRIORITY     ( tskIDLE_PRIORITY + 1UL )
#define MAIN_TASK_STACK_SIZE configMINIMAL_STACK_SIZE
#define PRIORITY_TASK_STACK_SIZE configMINIMAL_STACK_SIZE

void high_priority_task(__unused void *params) {
  if( semaphore != NULL )
    {
      /* See if we can obtain the semaphore. If the semaphore is not
	 available wait 10 ticks to see if it becomes free. */
      if( xSemaphoreTake( semaphore, ( TickType_t ) 10 ) == pdTRUE ) {
	printf(" High priority task was able to obtain the semaphore.");
	
	xSemaphoreGive( semaphore );
      }
      else {
	printf("unable to take semaphore");
      }      
    printf("work done and semaphore was released");      
    }
  while(1){
    
  }
}

void low_priority_task(__unused void *params) {
  if( semaphore != NULL )
    {
      /* See if we can obtain the semaphore. If the semaphore is not
	 available wait 10 ticks to see if it becomes free. */
      if( xSemaphoreTake( semaphore, ( TickType_t ) 4 ) == pdTRUE ) {
	printf(" Low priority task was  able to obtain the semaphore.");
	vTaskDelay(1000);
	xSemaphoreGive( semaphore );
      }
      else {
	/* We could not obtain the semaphore and can therefore not access
	   the shared resource safely. */
      }      
    printf("work done and semaphore was released");      
    }
  while(1){
    
  }
}

void supervisor_task(__unused void *params) {
  // create low priority task
  xTaskCreate(low_priority_task, "low_priority_thread",
                PRIORITY_TASK_STACK_SIZE, NULL, LOW_PRIORITY_TASK_PRIORITY, NULL);  
  vTaskDelay(1000);
  // create high priority task
  xTaskCreate(high_priority_task, "high_priority_thread",
                PRIORITY_TASK_STACK_SIZE, NULL, HIGH_PRIORITY_TASK_PRIORITY, NULL);

  char c;
    while(c = getchar()) {
        if (c <= 'z' && c >= 'a') putchar(c - 32);
        else if (c >= 'A' && c <= 'Z') putchar(c + 32);
        else putchar(c);
    }
}

int main( void )
{
    stdio_init_all();
    semaphore = xSemaphoreCreateBinary();
    if(!semaphore)
      {
	printf("Error creating semaphore. %s\n.", semaphore);
	return 0;
      }
    printf("semaphore was created successfully.");
    const char *rtos_name;
    rtos_name = "FreeRTOS";
    TaskHandle_t task;
    xTaskCreate(supervisor_task, "Supervisor_thread",
                MAIN_TASK_STACK_SIZE, NULL, MAIN_TASK_PRIORITY, &task);

    vTaskStartScheduler();
    return 0;
}
