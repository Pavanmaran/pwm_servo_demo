#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_log.h"

static const char *TAG = "PWM_BUZZER";

// --- Settings you can change ---
#define BUZZER_GPIO     GPIO_NUM_4          // pin the buzzer is connected to
#define PWM_RESOLUTION  LEDC_TIMER_10_BIT   // duty is a number from 0 to 1023
#define BUZZER_VOLUME   50                  // duty in percent (50 = loudest for a passive buzzer)
#define TEMPO_MS        400                 // length of one quarter note in ms

// Note frequencies in Hz (0 = silence / rest)
#define REST 0
#define C4 262
#define D4 294
#define E4 330
#define F4 349
#define G4 392
#define A4 440
#define B4 494
#define C5 523
#define D5 587
#define E5 659
#define F5 698
#define G5 784

// Note lengths (in ms), based on TEMPO_MS
#define Q   (TEMPO_MS)             // quarter
#define H   (TEMPO_MS * 2)         // half
#define DE  (TEMPO_MS * 3 / 4)     // dotted eighth
#define S   (TEMPO_MS / 4)         // sixteenth
#define DH  (TEMPO_MS * 3)         // dotted half

typedef struct {
    uint32_t freq_hz;
    uint32_t duration_ms;
} note_t;

static const note_t happy_birthday[] = {
    // Happy birthday to you
    {G4, DE}, {G4, S}, {A4, Q}, {G4, Q}, {C5, Q}, {B4, H},
    // Happy birthday to you
    {G4, DE}, {G4, S}, {A4, Q}, {G4, Q}, {D5, Q}, {C5, H},
    // Happy birthday dear (name)
    {G4, DE}, {G4, S}, {G5, Q}, {E5, Q}, {C5, Q}, {B4, Q}, {A4, Q},
    // Happy birthday to you
    {F5, DE}, {F5, S}, {E5, Q}, {C5, Q}, {D5, Q}, {C5, DH},
};

#define NOTE_COUNT (sizeof(happy_birthday) / sizeof(happy_birthday[0]))

// Play one note: change the PWM frequency, keep the duty at 50%
static void play_note(uint32_t freq_hz, uint32_t duration_ms)
{
    const int max_duty = (1 << PWM_RESOLUTION) - 1;

    if (freq_hz == REST) {
        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
        vTaskDelay(pdMS_TO_TICKS(duration_ms));
        return;
    }

    ledc_set_freq(LEDC_LOW_SPEED_MODE, LEDC_TIMER_0, freq_hz);
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, max_duty * BUZZER_VOLUME / 100);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);

    // Sound for 90% of the note, then a short silence so repeated notes are distinct
    vTaskDelay(pdMS_TO_TICKS(duration_ms * 9 / 10));
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
    vTaskDelay(pdMS_TO_TICKS(duration_ms / 10));
}

void app_main(void)
{
    // Step 1: set up the PWM timer (frequency is changed per note later)
    ledc_timer_config_t timer_config = {
        .speed_mode      = LEDC_LOW_SPEED_MODE,
        .timer_num       = LEDC_TIMER_0,
        .duty_resolution = PWM_RESOLUTION,
        .freq_hz         = C4,
        .clk_cfg         = LEDC_AUTO_CLK
    };
    ledc_timer_config(&timer_config);

    // Step 2: set up the PWM channel (starts silent)
    ledc_channel_config_t channel_config = {
        .gpio_num   = BUZZER_GPIO,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel    = LEDC_CHANNEL_0,
        .timer_sel  = LEDC_TIMER_0,
        .duty       = 0,
        .hpoint     = 0
    };
    ledc_channel_config(&channel_config);

    ESP_LOGI(TAG, "Buzzer ready on GPIO %d", BUZZER_GPIO);

    // Step 3: play the song, pause, repeat
    while (1) {
        ESP_LOGI(TAG, "Playing Happy Birthday");
        for (size_t i = 0; i < NOTE_COUNT; i++) {
            play_note(happy_birthday[i].freq_hz, happy_birthday[i].duration_ms);
        }
        vTaskDelay(pdMS_TO_TICKS(3000)); // wait 3 seconds before replaying
    }
}
