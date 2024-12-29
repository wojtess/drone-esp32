#ifndef PROTOCOL_H
#define PROTOCOL_H
#include <inttypes.h>
#include "core.h"
#include "esp_crc.h"

#define PROTOCOL_CRC_INIT 0xFFFFFFFF

// IEEE 802.11 Header (24 bytes)
// Defined in protocol.c as ieee80211header[]
typedef struct {
    uint8_t frame_control[2];
    uint8_t duration[2];
    uint8_t addr1[6];  // Destination MAC (broadcast)
    uint8_t addr2[6];  // Source MAC
    uint8_t addr3[6];  // BSSID
    uint8_t seq_ctrl[2];
} ieee80211_header_t;

// Protocol Header (7 bytes)
// Defined in protocol.c as magic_number[] + header_t
typedef struct {
    uint8_t magic[2];  // Fixed value 0x3C4A
    uint8_t id;        // Packet type identifier
    uint32_t crc;      // CRC32 of entire packet (header + payload)
} protocol_header_t;

// Complete Packet Structure
typedef struct {
    ieee80211_header_t ieee80211_header;
    protocol_header_t protocol_header;
    union {
        packet_out_0x01 type_0x01;
        packet_out_0x02 type_0x02;
        // Add other packet types as needed
    } payload;
} complete_packet_t;

extern uint8_t magic_number[2];  // Defined in protocol.c as {0x3C, 0x4A}

//camera data
typedef struct {
    long unsigned int index;
    long unsigned int len;
    uint8_t* data;
} packet_out_0x01;

int send_packet_0x01(packet_out_0x01 packet);

//current pitch yaw roll throttle and time of sent
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

//config packet
typedef struct {
    uint32_t frequency;
    bool packet_3;//if true then use data from packet_0x03, if false use data from packet_0x01
} packet_in_0x02;

void handle_packet_0x02(state_t* state, packet_in_0x02* packet);

//duty packet
typedef struct {
    uint32_t duty0;
    uint32_t duty1;
    uint32_t duty2;
    uint32_t duty3;
} packet_in_0x03;

void handle_packet_0x03(state_t* state, packet_in_0x03* packet);

typedef struct {
    uint8_t magic[2];
    uint8_t id;
    uint32_t crc; // CRC32 of the entire packet including header
} header_t;

int decode_and_handle_packet(state_t* state, header_t* header, void* buffer, int length);

#endif
