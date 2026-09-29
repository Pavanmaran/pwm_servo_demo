#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_log.h"

static const char *TAG = "PWM_SERVO";

// --- Settings you can change ---
#define SERVO_GPIO       GPIO_NUM_4          // pin the servo signal wire is connected to
#define PWM_FREQ_HZ      50                  // servos expect 50 Hz (20 ms period)
#define PWM_RESOLUTION   LEDC_TIMER_14_BIT   // duty is a number from 0 to 16383
#define SERVO_MIN_US     500                 // pulse width for 0 degrees (some servos: 1000)
#define SERVO_MAX_US     2500                // pulse width for 180 degrees (some servos: 2000)
#define SERVO_MAX_ANGLE  180

#define PERIOD_US        (1000000 / PWM_FREQ_HZ)

// Move the servo to an angle (0 - 180 degrees)
static void servo_set_angle(int angle)
{
    if (angle < 0) angle = 0;
    if (angle > SERVO_MAX_ANGLE) angle = SERVO_MAX_ANGLE;

    // angle -> pulse width in microseconds -> duty value
    uint32_t pulse_us = SERVO_MIN_US + (SERVO_MAX_US - SERVO_MIN_US) * angle / SERVO_MAX_ANGLE;
    uint32_t duty = (uint32_t)(((1 << PWM_RESOLUTION) - 1) * pulse_us / PERIOD_US);

    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
}

void app_main(void)
{
    // Step 1: set up the PWM timer (50 Hz for the servo)
    ledc_timer_config_t timer_config = {
        .speed_mode      = LEDC_LOW_SPEED_MODE,
        .timer_num       = LEDC_TIMER_0,
        .duty_resolution = PWM_RESOLUTION,
        .freq_hz         = PWM_FREQ_HZ,
        .clk_cfg         = LEDC_AUTO_CLK
    };
    ledc_timer_config(&timer_config);

    // Step 2: set up the PWM channel (which pin the servo is on)
    ledc_channel_config_t channel_config = {
        .gpio_num   = SERVO_GPIO,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel    = LEDC_CHANNEL_0,
        .timer_sel  = LEDC_TIMER_0,
        .duty       = 0,
        .hpoint     = 0
    };
    ledc_channel_config(&channel_config);

    ESP_LOGI(TAG, "Servo PWM started on GPIO %d at %d Hz", SERVO_GPIO, PWM_FREQ_HZ);

    // Step 3: demo loop
    while (1) {
        // Jump to 3 positions
        const int positions[] = {0, 90, 180, 90};
        for (int i = 0; i < 4; i++) {
            ESP_LOGI(TAG, "Angle: %d", positions[i]);
            servo_set_angle(positions[i]);
            vTaskDelay(pdMS_TO_TICKS(1000));
        }

        // Smooth sweep 0 -> 180 -> 0
        ESP_LOGI(TAG, "Sweeping");
        for (int angle = 0; angle <= 180; angle += 2) {
            servo_set_angle(angle);
            vTaskDelay(pdMS_TO_TICKS(20));
        }
        for (int angle = 180; angle >= 0; angle -= 2) {
            servo_set_angle(angle);
            vTaskDelay(pdMS_TO_TICKS(20));
        }
    }
}
