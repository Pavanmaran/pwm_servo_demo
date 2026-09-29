#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_log.h"

static const char *TAG = "PWM_LED";

// --- Settings you can change ---
#define LED_GPIO        GPIO_NUM_4   // pin the LED is connected to
#define PWM_FREQ_HZ     30000         // how fast the PWM switches on/off
#define PWM_RESOLUTION  LEDC_TIMER_10_BIT  // duty is a number from 0 to 1023
#define LED_BRIGHTNESS  100            // brightness in percent (0-100)

void app_main(void)
{
    /*
    // what is structure in c :
    struct is a user-defined data type in C that allows 
    you to group different types of variables together 
    under a single name. It is used to represent a 
    collection of related data items, which can be of
     different types. Each variable within a struct is 
     called a member or field.
    */
    // create strucutre
    // typedef struct {
    //     uint32_t sensorValue;
    //     uint8_t sensorPin;
    //     bool deviceState;         /*!< LEDC speed mode, high-speed mode (only exists on esp32) or low-speed mode */
    // } SensorData_t;


    // Step 1: set up the PWM timer (controls frequency)
    ledc_timer_config_t timer_config = {
        .speed_mode      = LEDC_LOW_SPEED_MODE,
        .timer_num       = LEDC_TIMER_0,
        .duty_resolution = PWM_RESOLUTION,
        .freq_hz         = PWM_FREQ_HZ,
        .clk_cfg         = LEDC_AUTO_CLK
    };
    ledc_timer_config(&timer_config);


    // Step 2: set up the PWM channel (controls which pin and brightness)
    int max_duty = (1 << PWM_RESOLUTION) - 1; // e.g. 1023 for 10-bit
    int duty = max_duty * LED_BRIGHTNESS / 100;

    ledc_channel_config_t channel_config = {
        .gpio_num   = LED_GPIO,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel    = LEDC_CHANNEL_0,
        .timer_sel  = LEDC_TIMER_0,
        .duty       = duty,
        .hpoint     = 0
    };
    ledc_channel_config(&channel_config);

    ESP_LOGI(TAG, "LED PWM started on GPIO %d", LED_GPIO);
    ESP_LOGI(TAG, "Frequency: %d Hz, Brightness: %d%%", PWM_FREQ_HZ, LED_BRIGHTNESS);

    // Step 3: just keep the program alive (PWM keeps running by itself)
    while (1) {
        // addjust the brightness/sound of LED/Buzzer from 
        // Low to High and High to Low with 1 second delay
        for (int i = 0; i <= 100; i++) {
            int duty = max_duty * i / 100;
            ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty);
            ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
            vTaskDelay(pdMS_TO_TICKS(10)); // 10 ms delay
        }
        for (int i = 100; i >= 0; i--) {
            int duty = max_duty * i / 100;
            ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty);
            ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
            vTaskDelay(pdMS_TO_TICKS(10)); // 10 ms delay       
       }
    }
}
