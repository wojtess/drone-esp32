#ifndef CORE_H
#define CORE_H
#include <time.h>
#include <inttypes.h>
#include <stdbool.h>
#include <freertos/FreeRTOS.h>

#include "motor.h"

typedef struct {
    double throttle;
    double pitch;
    double roll;
    double yaw;
} controls_t;

typedef struct {
    time_t last_sent_frame;
} timings_t;

typedef struct {
    uint32_t duty0;
    uint32_t duty1;
    uint32_t duty2;
    uint32_t duty3;
    uint32_t freq;
} pwmControls_t;

typedef struct {
    controls_t controls;
    timings_t timings;
    pwmControls_t pwmControls;
    bool pwmRaw;
    SemaphoreHandle_t mutex;
} state_t;

typedef struct {
    state_t* state;
    QueueHandle_t* sniffer_fifo;
} core_task_init_data;

void core_task(void*);//casted to core_task_init_data imiedlity

typedef struct {
    void* buf;//dynamic buffer that we read data from "move" to keep information what data was readed
    void* buf_original_pointer;//used for freeing memory
    int len;
} sniffer_data;


#endif