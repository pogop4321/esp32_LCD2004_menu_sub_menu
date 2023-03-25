#ifndef GLOB_VAR_H
#define GLOB_VAR_H

#define LCD_ADDR 0x27  // Change this to match your I2C LCD address
#define SDA_PIN 21     // GPIO number for the SDA pin
#define SCL_PIN 22     // GPIO number for the SCL pin
#define LCD_COLS 20    // Number of columns in your LCD
#define LCD_ROWS 4     // Number of rows in your LCD

#define UP_BUTTON_PIN 18
#define DOWN_BUTTON_PIN 19
#define SELECT_BUTTON_PIN 17
#define PUMP_PIN 33
#define LED_PIN 32
#define DEBOUNCE_TIME_MS 15

typedef enum {
    BUTTON_UP,
    BUTTON_DOWN,
    BUTTON_SELECT,
} button_event_t;

typedef enum {
    LOW,
    MIDDLE,
    HIGH,
    OFF,
} bright_state_t;


extern int stored_temperature;
extern int pump_time_state;
extern int led_time_state;

extern bright_state_t current_bright_state;
extern QueueHandle_t button_queue;
extern TimerHandle_t button_timer;
extern uint8_t last_button_states[3];

#endif