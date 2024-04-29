#ifndef CAMERA_H
#define CAMERA_H

#include "esp_camera.h"

esp_err_t init_camera(void);

void start_capture(uint8_t quality, jpg_out_cb cb, void * arg);
void stop_capture();

#endif