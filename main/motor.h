#ifndef MOTOR_H
#define MOTOR_H

#include "esp_err.h"

enum Motor {
    RIGHT_FRONT,
    LEFT_FRONT,
    RIGHT_BACK,
    LEFT_BACK
};

/**
 * @brief Initialize motors with default GPIO pins
 * @return ESP_OK on success, error code on failure
 */
esp_err_t initMotorsDefault();

/**
 * @brief Initialize motors with specified GPIO pins
 * @param motorA GPIO pin for motor A
 * @param motorB GPIO pin for motor B
 * @param motorC GPIO pin for motor C
 * @param motorD GPIO pin for motor D
 * @return ESP_OK on success, error code on failure
 * @retval ESP_ERR_INVALID_ARG if any pin is invalid
 * @retval ESP_FAIL if hardware initialization fails
 */
esp_err_t initMotors(int motorA, int motorB, int motorC, int motorD);

/**
 * @brief Set PWM value for a motor
 * @param motor Motor to control
 * @param pwm PWM value (0.0 to 1.0)
 */
void setPWMMotor(enum Motor motor, float pwm);

#endif // MOTOR_H
