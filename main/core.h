#ifndef CORE_H
#define CORE_H
#include <time.h>

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
    controls_t controls;
    timings_t timings;
} state_t;

#endif