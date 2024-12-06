#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/cyw43_arch.h"

#include <FreeRTOS.h>
#include "task.h"

#include "busy.h"

void busy_busy(__unused void *params)
{
    for (int i = 0; ; i++);
}

void busy_yield(__unused void *params)
{
    for (int i = 0; ; i++) {
        taskYIELD();
    }
}