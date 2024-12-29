#include "motor.h"

#include <stdio.h>
#include <math.h>
#include "driver/ledc.h"
#include "esp_err.h"

#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_DUTY_RES           LEDC_TIMER_13_BIT // Set duty resolution to 13 bits
#define LEDC_DUTY               (4095) // Set duty to 50%. ((2 ** 13) - 1) * 50% = 4095
// Note: 13-bit resolution means duty cycle values range from 0 to 8191 (2^13 - 1)
#define LEDC_FREQUENCY          (5000) // Frequency in Hertz. Set frequency at 5 kHz

esp_err_t initMotorsDefault() {
    return initMotors(12, 13, 15, 14);
}

esp_err_t initMotors(int motorA, int motorB, int motorC, int motorD) {
    esp_err_t err = ESP_OK;
    
    // Validate GPIO pins
    if (motorA < 0 || motorB < 0 || motorC < 0 || motorD < 0) {
        ESP_LOGE("MOTOR", "Invalid GPIO pin numbers");
        return ESP_ERR_INVALID_ARG;
    }

    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_MODE,
        .timer_num        = LEDC_TIMER,
        .duty_resolution  = LEDC_DUTY_RES,
        .freq_hz          = LEDC_FREQUENCY,
        .clk_cfg          = LEDC_AUTO_CLK
    };

    err = ledc_timer_config(&ledc_timer);
    if (err != ESP_OK) {
        ESP_LOGE("MOTOR", "Failed to configure LEDC timer: %s", esp_err_to_name(err));
        return err;
    }

     // Prepare and then apply the LEDC PWM channel configuration
    ledc_channel_config_t ledc_channel0 = {
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL_0,
        .timer_sel      = LEDC_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = motorA,
        .duty           = 0, // Set duty to 0%
        .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel0));

    // Prepare and then apply the LEDC PWM channel configuration
    ledc_channel_config_t ledc_channel1 = {
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL_1,
        .timer_sel      = LEDC_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = motorB,
        .duty           = 0, // Set duty to 0%
        .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel1));

    // Prepare and then apply the LEDC PWM channel configuration
    ledc_channel_config_t ledc_channel2 = {
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL_2,
        .timer_sel      = LEDC_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = motorC,
        .duty           = 0, // Set duty to 0%
        .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel2));

    // Prepare and then apply the LEDC PWM channel configuration
    ledc_channel_config_t ledc_channel3 = {
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL_3,
        .timer_sel      = LEDC_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = motorD,
        .duty           = 0, // Set duty to 0%
        .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel3));

}

#define MOTOR_TAG "MOTOR"

void setPWMMotor(enum Motor motor, float pwm) {
    if (pwm < 0.0f || pwm > 1.0f) {
        ESP_LOGE(MOTOR_TAG, "Invalid PWM value %f - must be between 0.0 and 1.0", pwm);
        return;
    }
    if (motor < 0 || motor > 3) {
        ESP_LOGE(MOTOR_TAG, "Invalid motor number %d", motor);
        return;
    }
    
    // Convert float PWM value (0.0 to 1.0) to 13-bit integer (0 to 8191)
    ledc_set_duty(LEDC_MODE, motor, (int)(8191 * pwm)); // 2^13 - 1 = 8191
    ledc_update_duty(LEDC_MODE, motor);
}
