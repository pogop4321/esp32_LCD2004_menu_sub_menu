#ifndef EXTERNAL_FUNC_H
#define EXTERNAL_FUNC_H

void save_pump_time_state_to_nvs(int pump_time_state);
int32_t read_pump_time_state_from_nvs();
void save_stored_temperature_to_nvs(int temperature);
int32_t load_stored_temperature_from_nvs();
void save_led_time_state_to_nvs(uint8_t state);
uint8_t load_led_time_state_from_nvs();
void save_bright_state_to_nvs(uint8_t state);
uint8_t load_bright_state_from_nvs();

void set_pump_state(uint8_t state);
void pump_task(void *arg);
void led_task(void *arg);

void main_menu();
void sub_menu(int main_selected);
void empty_function();
void print_menu(const char *menus[], int num_menus, int selected);
void button_timer_callback(TimerHandle_t xTimer);
void sub_menu_led();
void sub_menu_temperature();
void sub_menu_pump();


#endif