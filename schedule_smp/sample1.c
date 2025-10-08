/**
 * Copyright (c) 2022 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>

#include "pico/stdlib.h"
#include "pico/multicore.h"

#ifdef CYW43_WL_GPIO_LED_PIN
#include "pico/cyw43_arch.h"
#endif

#include "FreeRTOS.h"
#include "task.h"


// Delay between led blinking
#define LED_DELAY_MS 2000

// Priorities of our threads - higher numbers are higher priority
#define HIGH_TASK_PRIORITY    ( tskIDLE_PRIORITY + 3UL )
#define MID_TASK_PRIORITY     ( tskIDLE_PRIORITY + 2UL )
#define LOW_TASK_PRIORITY     ( tskIDLE_PRIORITY + 1UL )

// Stack sizes of our threads in words (4 bytes)
#define TASK_STACK_SIZE configMINIMAL_STACK_SIZE


// Turn led on or off
static void
pico_set_led(bool led_on) {
#if defined PICO_DEFAULT_LED_PIN
    gpio_put(PICO_DEFAULT_LED_PIN, led_on);
#elif defined(CYW43_WL_GPIO_LED_PIN)
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, led_on);
#endif
}


// Initialise led
static void
pico_init_led(void) {
#if defined PICO_DEFAULT_LED_PIN
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
#elif defined(CYW43_WL_GPIO_LED_PIN)
    hard_assert(cyw43_arch_init() == PICO_OK);
    pico_set_led(false); // make sure cyw43 is started
#endif
}

TaskHandle_t btask_m_handle;
TaskHandle_t btask_l_handle;
TaskHandle_t otask_h_handle;

int 
GetCoreID(void) 
{
    return portGET_CORE_ID();
}

char message;
repeating_timer_t rtimer;


bool 
timer_callback( repeating_timer_t *rt )
{
    printf("timer_callback : Start! on core %d\n", GetCoreID());

    return true;
}

void
btask_m(__unused void *params)
{
    int last_core_id = -1;
    int cur_core_id;
    int c;

    printf("btask_m : Start!\n");
    printf("btask_m : Press any key to resume otask_h\n");

    message = 0;

    while(true) {
        printf("btask_m : running on core %d\n", GetCoreID());
        c = getchar_timeout_us(0);

        if (c == 'w') {
            printf("btask_m : Set otask_h to core %d\n", GetCoreID());
            vTaskCoreAffinitySet(otask_h_handle, 1 << GetCoreID());
            vTaskResume(otask_h_handle);
        }
        else if (c == 'd') {
            printf("btask_m : vTaskPreemptionDisable() \n");
            message = 'd';
            vTaskPreemptionDisable(NULL);
        }
        else if (c == 'e') {
            printf("btask_m : vTaskPreemptionEnable() \n");
            message = 'e';
            vTaskPreemptionEnable(NULL);
        }
        else if (c == 's') {
            printf("btask_m : vTaskSuspendAll() \n");
            vTaskSuspendAll();
        }
        else if (c == 'r') {
            printf("btask_m : xTaskResumeAll() \n");
            xTaskResumeAll();
        }
        else if (c == 'c') {
            printf("btask_m : Start Cyclic Timer \n");
            add_repeating_timer_ms(500, &timer_callback, NULL, &rtimer);
        }
        else if (c == 'C') {
            printf("btask_m : Cancel Cyclic Timer \n");
            cancel_repeating_timer(&rtimer);
        }
        else if (c == '1') {
            printf("btask_m : Set Core Affinity to Core 0\n");
            vTaskCoreAffinitySet(NULL, 1 << 0);
        }        
         else if (c == '2') {
            printf("btask_m : Set Core Affinity to Core 1\n");
            vTaskCoreAffinitySet(NULL, 1 << 1);
        }       
        busy_wait_ms(500);
        //sleep_ms(500);
    }
}


void
btask_l(__unused void *params)
{
    int last_core_id = -1;
    int cur_core_id;

    printf("btask_l : Start!\n");
    
    while(true) {
        printf("btask_l : running on core %d\n", GetCoreID());

        if (message == 'd') {
            printf("btask_l : vTaskPreemptionDisable() \n");
            vTaskPreemptionDisable(NULL);
        }
        else if (message == 'e') {
            printf("btask_l : vTaskPreemptionEnable() \n");
            vTaskPreemptionEnable(NULL);
        }

        message = 0;

        busy_wait_ms(500);

    }
}
    
void
otask_h(__unused void *params)
{

    printf("otask_h :Start!\n");

    while(true) {
        printf("otask_h : Suspend on core %d\n", GetCoreID());
        
        vTaskSuspend(NULL);

        printf("otask_h : Resume on core %d\n", GetCoreID());

        busy_wait_ms(1000);
        printf("otask_h : On core %d\n", GetCoreID());
        busy_wait_ms(1000);
        printf("otask_h : On core %d\n", GetCoreID());
        busy_wait_ms(1000);
        printf("otask_h : On core %d\n", GetCoreID());
        busy_wait_ms(1000);
    }
}


int
main( void )
{
    stdio_init_all();

    /* Configure the hardware ready to run the demo. */
    const char *rtos_name;
    rtos_name = "FreeRTOS SMP";

    printf("Starting %s on both cores:\n", rtos_name);

    xTaskCreate(btask_m, "btask_m", TASK_STACK_SIZE, NULL, MID_TASK_PRIORITY, &btask_m_handle);
    xTaskCreate(btask_l, "btask_l", TASK_STACK_SIZE, NULL, LOW_TASK_PRIORITY, &btask_l_handle);
    xTaskCreate(otask_h, "otask_h", TASK_STACK_SIZE, NULL, HIGH_TASK_PRIORITY, &otask_h_handle);

    pico_init_led();

    /* Start the tasks and timer running. */
    vTaskStartScheduler();
    
    return 0;
}
