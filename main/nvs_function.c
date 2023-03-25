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

void save_pump_time_state_to_nvs(int pump_time_state) {
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &nvs_handle);

    if (err != ESP_OK) {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    } else {
        err = nvs_set_i32(nvs_handle, "pump_t_state", pump_time_state);
        printf((err != ESP_OK) ? "Failed to write pump_time_state to NVS!\n" : "Updated pump_time_state in NVS\n");

        nvs_commit(nvs_handle);
    }

    nvs_close(nvs_handle);
}

int32_t read_pump_time_state_from_nvs() {
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open("storage", NVS_READONLY, &nvs_handle);

    int32_t pump_time_state = 0; // Default value if no value is found in NVS

    if (err == ESP_OK) {
        err = nvs_get_i32(nvs_handle, "pump_t_state", &pump_time_state);
        if (err == ESP_ERR_NVS_NOT_FOUND) {
            printf("The time_state is not initialized yet!\n");
        } else if (err == ESP_OK) {
            printf("Read pump_time_state from NVS: %ld\n", pump_time_state);
        } else {
            printf("Error (%s) reading pump_time_state from NVS!\n", esp_err_to_name(err));
        }
        nvs_close(nvs_handle);
    } else {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    }

    return pump_time_state;
}


void save_stored_temperature_to_nvs(int temperature) {
    nvs_handle_t my_handle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
        return;
    }

    err = nvs_set_i32(my_handle, "st_temperature", temperature);
    if (err != ESP_OK) {
        printf("Error (%s) saving stored_temperature!\n", esp_err_to_name(err));
    }

    err = nvs_commit(my_handle);
    if (err != ESP_OK) {
        printf("Error (%s) committing changes to NVS!\n", esp_err_to_name(err));
    }

    nvs_close(my_handle);
}

int32_t load_stored_temperature_from_nvs() {
    nvs_handle_t my_handle;
    esp_err_t err = nvs_open("storage", NVS_READONLY, &my_handle);
    if (err != ESP_OK) {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
        return 20; // Set a default value here
    }

    int32_t temperature = 0;
    err = nvs_get_i32(my_handle, "st_temperature", &temperature);
    if (err != ESP_OK) {
        if (err == ESP_ERR_NVS_NOT_FOUND) {
            printf("stored_temperature not found, using default value\n");
            temperature = 20; // Set a default value here
        } else {
            printf("Error (%s) loading stored_temperature!\n", esp_err_to_name(err));
        }
    }

    nvs_close(my_handle);
    return temperature;
}


void save_led_time_state_to_nvs(uint8_t state) {
    nvs_handle_t my_handle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
        return;
    }

    err = nvs_set_u8(my_handle, "led_time_state", state);
    if (err != ESP_OK) {
        printf("Error (%s) saving led_time_state!\n", esp_err_to_name(err));
    }

    err = nvs_commit(my_handle);
    if (err != ESP_OK) {
        printf("Error (%s) committing changes to NVS!\n", esp_err_to_name(err));
    }

    nvs_close(my_handle);
}

uint8_t load_led_time_state_from_nvs() {
    nvs_handle_t my_handle;
    esp_err_t err = nvs_open("storage", NVS_READONLY, &my_handle);
    if (err != ESP_OK) {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
        return 0;
    }

    uint8_t state = 0;
    err = nvs_get_u8(my_handle, "led_time_state", &state);
    if (err != ESP_OK) {
        printf("Error (%s) loading led_time_state!\n", esp_err_to_name(err));
    }

    nvs_close(my_handle);
    return state;
}

void save_bright_state_to_nvs(uint8_t state) {
    nvs_handle_t my_handle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
        return;
    }

    err = nvs_set_u8(my_handle, "bright_state", state);
    if (err != ESP_OK) {
        printf("Error (%s) saving bright_state!\n", esp_err_to_name(err));
    }

    err = nvs_commit(my_handle);
    if (err != ESP_OK) {
        printf("Error (%s) committing changes to NVS!\n", esp_err_to_name(err));
    }

    nvs_close(my_handle);
}

uint8_t load_bright_state_from_nvs() {
    nvs_handle_t my_handle;
    esp_err_t err = nvs_open("storage", NVS_READONLY, &my_handle);
    if (err != ESP_OK) {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
        return 0;
    }

    uint8_t state = 0;
    err = nvs_get_u8(my_handle, "bright_state", &state);
    if (err != ESP_OK) {
        printf("Error (%s) loading bright_state!\n", esp_err_to_name(err));
    }

    nvs_close(my_handle);
    return state;
}