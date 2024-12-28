#include <protocol.h>

#include "endianness.h"
#include "esp_wifi.h"
#include <stdio.h>
#include <inttypes.h>
#include <string.h>

uint8_t magic_number[]= {
   0x3C, 0x4A, // Magic number to identify our protocol packets
   // This helps distinguish our packets from other WiFi traffic
};

//SEND
uint8_t ieee80211header[] = {
    //IEEE802.11 header
    0x48, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,//addr1
    0x13, 0x22, 0x33, 0x44, 0x55, 0x66,//addr2
    0x13, 0x22, 0x33, 0x44, 0x55, 0x66,//addr3
    0x00, 0x00,//duration or seq number, idk
    //END IEEE802.11 header
};

int send_packet_0x02(packet_out_0x02 packet) {
    // Packet structure: [IEEE802.11 header][magic number][packet ID][payload]
    size_t buf_size = sizeof(ieee80211header) + sizeof(magic_number) + 1 + sizeof(packet);
    char* buf = malloc(buf_size);
    if (!buf) {
        return -1;
    }

    memcpy(buf, &ieee80211header, sizeof(ieee80211header));
    memcpy(buf + sizeof(ieee80211header), &magic_number, sizeof(magic_number));
    buf[sizeof(ieee80211header) + sizeof(magic_number)] = 0x02;
    memcpy(buf + sizeof(ieee80211header) + sizeof(magic_number) + /*packet id*/1, &packet, sizeof(packet));

    // Use ESP32's low-level WiFi API to send raw 802.11 packets
    extern int esp_wifi_80211_tx_mod(wifi_interface_t ifx, const void *buffer, int len, bool en_sys_seq);

    int err = esp_wifi_80211_tx_mod(WIFI_IF_STA, buf, sizeof(buf), false);
    if(err != ESP_OK) {
        printf("error while sending packet 0x02");
        return err;
    }
    return ESP_OK;
}

int encode_packet_out_0x01(packet_out_0x01 packet, void** buf, int* len) {
    // Packet format: [4 bytes index][4 bytes length][variable data]
    *len = 4/*index*/ + 4/*len*/ + packet.len /*data*/;
    *buf = malloc(*len);
    if(*buf == NULL) {
        return -1;
    }
    memcpy((uint8_t*)*buf + 4 + 4, packet.data, packet.len);
    return 0;
}

//TODO: test
int send_packet_0x01(packet_out_0x01 packet) {
    void* buf;
    int len;
    int err = encode_packet_out_0x01(packet, &buf, &len);
    if(err != 0) {
        printf("error while encoding packet_0x01: %d\n", err);
        return -1;
    }

    int packet_ieee80211_len = sizeof(ieee80211header) + sizeof(magic_number) + 1 /*packet id*/ + len;
    void* packet_ieee80211 = malloc(packet_ieee80211_len);
    if(packet_ieee80211 == 0) {
        printf("error while allocationg packet ieee80211 for packet_0x01\n");
        free(buf);
        return -2;
    }

    memcpy(packet_ieee80211, &ieee80211header, sizeof(ieee80211header));
    memcpy(packet_ieee80211 + sizeof(ieee80211header), &magic_number, sizeof(magic_number));
    ((uint8_t*)packet_ieee80211)[sizeof(ieee80211header) + sizeof(magic_number)] = 0x01;//packet id
    memcpy(packet_ieee80211 + (sizeof(ieee80211header) + sizeof(magic_number) + 1/*packet id*/), buf, len);

    free(buf);

    extern int esp_wifi_80211_tx_mod(wifi_interface_t ifx, const void *buffer, int len, bool en_sys_seq);

    err = esp_wifi_80211_tx_mod(WIFI_IF_STA, packet_ieee80211, packet_ieee80211_len, false);

    free(packet_ieee80211);
    if(err != ESP_OK) {
        printf("error while sending packet0x01: %s\n", esp_err_to_name(err));
        return -3;
    }

    return 0;
}

//END SEND
//RECIVE
void handle_packet_0x01(state_t* state, packet_in_0x01* packet) {
    // Convert big-endian values from network to host byte order
    uint64_t throttleRaw = end_be64toh(packet->throttle);
    state->controls.throttle = (double)throttleRaw;

    uint64_t pitchRaw = end_be64toh(packet->pitch);
    memcpy(&state->controls.pitch, &pitchRaw, sizeof(state->controls.pitch));

    uint64_t rollRaw = end_be64toh(packet->roll);
    memcpy(&state->controls.roll, &rollRaw, sizeof(state->controls.roll));

    uint64_t yawRaw = end_be64toh(packet->yaw);
    memcpy(&state->controls.yaw, &yawRaw, sizeof(state->controls.yaw));

    printf("throttle: %lf, pitch: %lf, roll: %lf, yaw: %lf\n",
        state->controls.throttle,
        state->controls.pitch,
        state->controls.roll,
        state->controls.yaw
    );
}

void handle_packet_0x02(state_t* state, packet_in_0x02* packet) {
    state->pwmRaw = packet->packet_3;
    state->pwmControls.freq = packet->frequency;
}

void handle_packet_0x03(state_t* state, packet_in_0x03* packet) {
    state->pwmControls.duty0 = packet->duty0;
    state->pwmControls.duty1 = packet->duty1;
    state->pwmControls.duty2 = packet->duty2;
    state->pwmControls.duty3 = packet->duty3;
}

int decode_and_handle_packet(state_t* state, header_t* header, void* buffer, int length) {
    // Dispatch packet to appropriate handler based on packet ID
    switch (header->id)
    {
    case 1: {
        if(length - (int)sizeof(packet_in_0x01) < 0) {
            return -2;
        }
        handle_packet_0x01(state, (packet_in_0x01*)buffer);
        buffer = (uint8_t*)buffer + sizeof(packet_in_0x01);
        length -= sizeof(packet_in_0x01);
        break;
    }
    case 2: {
        if(length - (int)sizeof(packet_in_0x02) < 0) {
            return -3;
        }
        handle_packet_0x02(state, (packet_in_0x02*)buffer);
        buffer = (uint8_t*)buffer + sizeof(packet_in_0x02);
        length -= sizeof(packet_in_0x02);
        break;
    }
    case 3: {
        if(length - (int)sizeof(packet_in_0x03) < 0) {
            return -4;
        }
        handle_packet_0x03(state, (packet_in_0x03*)buffer);
        buffer = (uint8_t*)buffer + sizeof(packet_in_0x03);
        length -= sizeof(packet_in_0x03);
        break;
    }
    default:
        return -1;
        break;
    }
    return 0;
}
