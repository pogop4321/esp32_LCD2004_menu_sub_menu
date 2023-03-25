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


int stored_temperature = 20;
int pump_time_state = 5; // Index of the "OFF" state
int led_time_state = 0;

const char *bright_state_to_string(bright_state_t state);

bright_state_t current_bright_state = OFF;
QueueHandle_t button_queue;
TimerHandle_t button_timer;
uint8_t last_button_states[3] = {1, 1, 1};




void app_main() {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    current_bright_state = load_bright_state_from_nvs();
    led_time_state = load_led_time_state_from_nvs();
    stored_temperature = load_stored_temperature_from_nvs();
    pump_time_state = read_pump_time_state_from_nvs();

    LCD_init(LCD_ADDR, SDA_PIN, SCL_PIN, LCD_COLS, LCD_ROWS);
    button_queue = xQueueCreate(10, sizeof(button_event_t));

    gpio_set_direction(UP_BUTTON_PIN, GPIO_MODE_INPUT);
    gpio_set_direction(DOWN_BUTTON_PIN, GPIO_MODE_INPUT);
    gpio_set_direction(SELECT_BUTTON_PIN, GPIO_MODE_INPUT);

    gpio_set_pull_mode(UP_BUTTON_PIN, GPIO_PULLDOWN_ONLY);
    gpio_set_pull_mode(DOWN_BUTTON_PIN, GPIO_PULLDOWN_ONLY);
    gpio_set_pull_mode(SELECT_BUTTON_PIN, GPIO_PULLDOWN_ONLY);

    button_timer = xTimerCreate("button_timer", pdMS_TO_TICKS(DEBOUNCE_TIME_MS), pdTRUE, (void *)0, button_timer_callback);
    xTimerStart(button_timer, 0);
    xTaskCreate(pump_task, "pump_task", 2048, NULL, tskIDLE_PRIORITY, NULL);
    xTaskCreate(led_task, "led_task", 2048, NULL, tskIDLE_PRIORITY, NULL);

    const char *menus[] = {"1.LED", "2.TEMPERATURE", "3.PUMP"};
    int selected = 0;
    button_event_t event;

    // Print the main menu first
    print_menu(menus, sizeof(menus) / sizeof(menus[0]), selected);
    LCD_setCursor(1, 3);
    LCD_writeStr("NOW");
    while (1) {
        if (xQueueReceive(button_queue, &event, portMAX_DELAY)) {
            switch (event) {
                case BUTTON_UP:
                    selected = (selected - 1 + sizeof(menus) / sizeof(menus[0])) % (sizeof(menus) / sizeof(menus[0]));
                    break;
                case BUTTON_DOWN:
                    selected = (selected + 1) % (sizeof(menus) / sizeof(menus[0]));
                    break;
                case BUTTON_SELECT:
                    //printf("Entering: %s\n", menus[selected]);
                    LCD_clearScreen();
                    LCD_setCursor(1, 3);
                    LCD_writeStr("NOW");
                    sub_menu(selected);
                    break;
            }
            print_menu(menus, sizeof(menus) / sizeof(menus[0]), selected);
        }
    }
}

void sub_menu(int main_selected) {
    LCD_clearScreen();
    LCD_setCursor(1, 3);
    LCD_writeStr("NOW");
    switch (main_selected) {
        case 0:
            sub_menu_led();
            break;
        case 1:
            sub_menu_temperature();
            break;
        case 2:
            sub_menu_pump();
            break;
    }
}

void print_menu(const char *menus[], int num_menus, int selected) {
    for (int i = 0; i < num_menus; i++) {
        LCD_setCursor(0, i);
        if (i == selected) {
            LCD_writeStr("*");
        } else {
            LCD_writeStr(" ");
        }
    LCD_writeStr(menus[i]);
    }
}

void button_timer_callback(TimerHandle_t xTimer) {
    uint8_t button_states[] = {
        gpio_get_level(UP_BUTTON_PIN),
        gpio_get_level(DOWN_BUTTON_PIN),
        gpio_get_level(SELECT_BUTTON_PIN)
    };

    for (int i = 0; i < 3; i++) {
        if (last_button_states[i] != button_states[i] && button_states[i] == 1) {
            button_event_t event = (button_event_t)i;
            xQueueSend(button_queue, &event, 0);
        }
        last_button_states[i] = button_states[i];
    }
}


const char *bright_state_to_string(bright_state_t state) {
    switch (state) {
        case LOW:
            return "LOW   ";
        case MIDDLE:
            return "MIDDLE";
        case HIGH:
            return "HIGH  ";
        case OFF:
            return "OFF   ";
    }
    return "INVALID";
}

void sub_menu_led() {
    const char *sub_menus[] = {"1.BRIGHT", "2.TIME", "3.BACK"};
    int selected = 0;
    button_event_t event;

    // int led_state = 0;
    const char *led_states[] = {"12hON 12hOFF", "10hON 14hOFF", "8hON 16hOFF", "ALWAYS ON"};

    // Display the stored LED state when entering the submenu
    LCD_setCursor(10, 0);
    LCD_writeStr(bright_state_to_string(current_bright_state));

    LCD_setCursor(8, 1);
    LCD_writeStr(led_states[led_time_state]);
    
    print_menu(sub_menus, sizeof(sub_menus) / sizeof(sub_menus[0]), selected);

    while (1) {
        if (xQueueReceive(button_queue, &event, portMAX_DELAY)) {
            switch (event) {
                case BUTTON_UP:
                    selected = (selected - 1 + sizeof(sub_menus) / sizeof(sub_menus[0])) % (sizeof(sub_menus) / sizeof(sub_menus[0]));
                    break;
                case BUTTON_DOWN:
                    selected = (selected + 1) % (sizeof(sub_menus) / sizeof(sub_menus[0]));
                    break;
                case BUTTON_SELECT:
                    if (selected == 0) {
                        current_bright_state = (current_bright_state + 1) % 4; // Cycle through the LED states and store the state
                        save_bright_state_to_nvs(current_bright_state);
                        LCD_setCursor(10, 0);
                        LCD_writeStr(bright_state_to_string(current_bright_state));
                    } else if (selected == 1) {
                        led_time_state = (led_time_state + 1) % (sizeof(led_states) / sizeof(led_states[0]));
                        save_led_time_state_to_nvs(led_time_state); // Save the new value to NVS
                        LCD_setCursor(8, 1);
                        LCD_writeStr("             "); // Clear the previous state
                        LCD_setCursor(8, 1);
                        LCD_writeStr(led_states[led_time_state]);
                    } else if (selected == 2) {
                        LCD_clearScreen();
                        LCD_setCursor(1, 3);
                        LCD_writeStr("NOW");
                        return;
                    }
                    break;
            }
            print_menu(sub_menus, sizeof(sub_menus) / sizeof(sub_menus[0]), selected);
        }
    }
}

void sub_menu_temperature() {
    const char *sub_menus[] = {"1.UP", "2.DOWN", "3.BACK"};
    int selected = 0;
    button_event_t event;

    // Add these lines to display the initial messages
    int temperature = stored_temperature; // Set initial temperature value
    LCD_setCursor(13, 0);
    LCD_writeStr("FAN ON");
    LCD_setCursor(13, 1);
    LCD_writeStr("AT   *C");
    LCD_setCursor(16, 1);
    char str[10];
    snprintf(str, sizeof(str) - 1, "%d", temperature);
    LCD_writeStr(str);

    print_menu(sub_menus, sizeof(sub_menus) / sizeof(sub_menus[0]), selected);

    while (1) {
        if (xQueueReceive(button_queue, &event, portMAX_DELAY)) {
            switch (event) {
                case BUTTON_UP:
                    selected = (selected - 1 + sizeof(sub_menus) / sizeof(sub_menus[0])) % (sizeof(sub_menus) / sizeof(sub_menus[0]));
                    break;
                case BUTTON_DOWN:
                    selected = (selected + 1) % (sizeof(sub_menus) / sizeof(sub_menus[0]));
                    break;
                case BUTTON_SELECT:
                    if (selected == 0) {
                        // Implement the Temp. up functionality here
                        if (temperature < 40) {
                            temperature++;
                            stored_temperature = temperature;
                            save_stored_temperature_to_nvs(temperature); // Save the new value to NVS
                            LCD_setCursor(16, 1);
                            snprintf(str, sizeof(str) - 1, "%d", temperature);
                            LCD_writeStr(str);
                        }
                    } else if (selected == 1) {
                        // Implement the Temp. down functionality here
                         if (temperature > 10) {
                            temperature--;
                            stored_temperature = temperature;
                            save_stored_temperature_to_nvs(temperature); // Save the new value to NVS
                            LCD_setCursor(16, 1);
                            snprintf(str, sizeof(str) - 1, "%d", temperature);
                            LCD_writeStr(str);
                        }
                    } else if (selected == 2) {
                        LCD_clearScreen();
                        LCD_setCursor(1, 3);
                        LCD_writeStr("NOW");
                        return;
                    }
                    break;
            }
            print_menu(sub_menus, sizeof(sub_menus) / sizeof(sub_menus[0]), selected);
        }
    }
}


void sub_menu_pump() {
    const char *sub_menus[] = {"1.TIME", "2.BACK"};
    int selected = 0;
    button_event_t event;

    // Declare and initialize the time_value variable
    static char time_value[10] = "OFF";
    const char *pump_time_states[] = {"Always   ", "3min/0.5h", "10min/0.5h", "3min/1h", "10min/1h", "OFF      "};
    strncpy(time_value, pump_time_states[pump_time_state], sizeof(time_value) - 1);
    time_value[sizeof(time_value) - 1] = '\0';
    // Display the "PUMP ON" message
    LCD_setCursor(9, 0);
    LCD_writeStr("PUMP TURN");

    // Display the time_value
    LCD_setCursor(9, 1);
    LCD_writeStr(time_value);

    print_menu(sub_menus, sizeof(sub_menus) / sizeof(sub_menus[0]), selected);

    while (1) {
        if (xQueueReceive(button_queue, &event, portMAX_DELAY)) {
            switch (event) {
                case BUTTON_UP:
                    selected = (selected - 1 + sizeof(sub_menus) / sizeof(sub_menus[0])) % (sizeof(sub_menus) / sizeof(sub_menus[0]));
                    break;
                case BUTTON_DOWN:
                    selected = (selected + 1) % (sizeof(sub_menus) / sizeof(sub_menus[0]));
                    break;
                case BUTTON_SELECT:
                    if (selected == 0) {
                        // Change the time_state and update the time_value
                        pump_time_state = (pump_time_state + 1) % (sizeof(pump_time_states) / sizeof(pump_time_states[0]));
                        strncpy(time_value, pump_time_states[pump_time_state], sizeof(time_value) - 1);
                        time_value[sizeof(time_value) - 1] = '\0';
                        save_pump_time_state_to_nvs(pump_time_state);
                        // Update the displayed time_value
                        LCD_setCursor(9, 1);
                        LCD_writeStr("         "); // Clear the previous time_value
                        LCD_setCursor(9, 1);
                        LCD_writeStr(time_value);
                    } else if (selected == 1) {
                        LCD_clearScreen();
                        LCD_setCursor(1, 3);
                        LCD_writeStr("NOW");
                        return;
                    }
                    break;
            }
            print_menu(sub_menus, sizeof(sub_menus) / sizeof(sub_menus[0]), selected);
        }
    }
}