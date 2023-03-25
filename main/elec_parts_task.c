#pragma GCC diagnostic ignored "-Wformat-truncation"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/timers.h"
#include "driver/gpio.h"
#include "HD44780.h"
#include <string.h>
#include "nvs_flash.h"
#include "nvs.h"
#include "GLOB_VAR.h"
#include "EXTERNAL_FUNC.h"


void set_pump_state(uint8_t state) {
    gpio_set_level(PUMP_PIN, state);
}

void pump_task(void *arg) {
    TickType_t xLastWakeTime;
    const TickType_t xFrequency = pdMS_TO_TICKS(1000); // Check the time state every second
    xLastWakeTime = xTaskGetTickCount();

    gpio_set_direction(PUMP_PIN, GPIO_MODE_OUTPUT);
    set_pump_state(0); // Initial state is off

    while (1) {
        vTaskDelayUntil(&xLastWakeTime, xFrequency);

        switch (pump_time_state) {
            case 0: // Always on
                set_pump_state(1);
                break;
            case 1: // 3min/0.5h
                gpio_set_level(GPIO_NUM_33, 1);
                vTaskDelay(pdMS_TO_TICKS(3 * 60 * 1000)); // 3 minutes on
                gpio_set_level(GPIO_NUM_33, 0);
                vTaskDelay(pdMS_TO_TICKS(27 * 60 * 1000)); // 27 minutes off
                break;
            case 2: // 10min/0.5h
                gpio_set_level(GPIO_NUM_33, 1);
                vTaskDelay(pdMS_TO_TICKS(10 * 60 * 1000)); // 10 minutes on
                gpio_set_level(GPIO_NUM_33, 0);
                vTaskDelay(pdMS_TO_TICKS(20 * 60 * 1000)); // 20 minutes off
                break;
            case 3: // 3min/1h
                gpio_set_level(GPIO_NUM_33, 1);
                vTaskDelay(pdMS_TO_TICKS(3 * 60 * 1000)); // 3 minutes on
                gpio_set_level(GPIO_NUM_33, 0);
                vTaskDelay(pdMS_TO_TICKS(57 * 60 * 1000)); // 57 minutes off
                break;
            case 4: // 10min/1h
                gpio_set_level(GPIO_NUM_33, 1);
                vTaskDelay(pdMS_TO_TICKS(10 * 60 * 1000)); // 10 minutes on
                gpio_set_level(GPIO_NUM_33, 0);
                vTaskDelay(pdMS_TO_TICKS(50 * 60 * 1000)); // 50 minutes off
                break;
            case 5: // Off
            default:
                set_pump_state(0);
                break;
        }
    }
}


void led_task(void *arg) {
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(LED_PIN, 0); // Initial state is off

    while (1) {
        switch (led_time_state) {
            case 0: // 12hON 12hOFF
                gpio_set_level(LED_PIN, 1);
                vTaskDelay(pdMS_TO_TICKS(12 * 60 * 60 * 1000)); // 12 hours on
                gpio_set_level(LED_PIN, 0);
                vTaskDelay(pdMS_TO_TICKS(12 * 60 * 60 * 1000)); // 12 hours off
                break;
            case 1: // 10hON 14hOFF
                gpio_set_level(LED_PIN, 1);
                vTaskDelay(pdMS_TO_TICKS(10 * 60 * 60 * 1000)); // 10 hours on
                gpio_set_level(LED_PIN, 0);
                vTaskDelay(pdMS_TO_TICKS(14 * 60 * 60 * 1000)); // 14 hours off
                break;
            case 2: // 8hON 16hOFF
                gpio_set_level(LED_PIN, 1);
                vTaskDelay(pdMS_TO_TICKS(8 * 60 * 60 * 1000)); // 8 hours on
                gpio_set_level(LED_PIN, 0);
                vTaskDelay(pdMS_TO_TICKS(16 * 60 * 60 * 1000)); // 16 hours off
                break;
            case 3: // ALWAYS ON
                gpio_set_level(LED_PIN, 1);
                vTaskDelay(portMAX_DELAY); // Keep the LED on
                break;
            default:
                gpio_set_level(LED_PIN, 0);
                vTaskDelay(portMAX_DELAY); // Keep the LED off
                break;
        }
    }
}
