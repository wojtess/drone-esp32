#ifndef PROTOCOL_H
#define PROTOCOL_H
#include <inttypes.h>
#include "core.h"

typedef struct {
    long unsigned int index;
    long unsigned int len;
    uint8_t* data;
} packet_out_0x01;

int send_packet_0x01(packet_out_0x01 packet);

typedef struct {
    int32_t time;
    
    uint64_t throttle;
    uint64_t pitch;
    uint64_t roll;
    uint64_t yaw;
} packet_out_0x02;

int send_packet_0x02(packet_out_0x02 packet);

typedef struct {
    uint64_t throttle;
    uint64_t pitch;
    uint64_t roll;
    uint64_t yaw;
} packet_in_0x01;

void handle_packet_0x01(state_t* state, packet_in_0x01* packet);

typedef struct {
    uint8_t magic[2];
    uint8_t id;
} header_t;

int decode_and_handle_packet(state_t* state, header_t* header, void* buffer, int length);

#endif