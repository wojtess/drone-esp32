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

#define PROTOCOL_TAG "PROTOCOL"

int send_packet_0x02(packet_out_0x02 packet) {
    // Packet structure: [IEEE802.11 header][magic number][packet ID][CRC32][payload]
    size_t buf_size = sizeof(ieee80211header) + sizeof(magic_number) + 1 + sizeof(uint32_t) + sizeof(packet);
    char* buf = malloc(buf_size);
    if (!buf) {
        ESP_LOGE(PROTOCOL_TAG, "Failed to allocate %d bytes for packet buffer", buf_size);
        return PROTOCOL_ERR_ALLOC;
    }

    memcpy(buf, &ieee80211header, sizeof(ieee80211header));
    // Build packet
    uint8_t* p = buf + sizeof(ieee80211header);
    protocol_header_t header = {
        .magic = {magic_number[0], magic_number[1]},
        .id = 0x02,
        .crc = 0 // Will be calculated below
    };
    
    // Calculate CRC over payload
    header.crc = esp_crc32_le(PROTOCOL_CRC_INIT, (uint8_t*)&packet, sizeof(packet));
    
    // Copy header
    memcpy(p, &header, sizeof(header));
    p += sizeof(header);
    
    memcpy(p, &packet, sizeof(packet));

    // Use ESP32's low-level WiFi API to send raw 802.11 packets
    extern int esp_wifi_80211_tx_mod(wifi_interface_t ifx, const void *buffer, int len, bool en_sys_seq);

    int err = esp_wifi_80211_tx_mod(WIFI_IF_STA, buf, buf_size, false);
    if(err != ESP_OK) {
        ESP_LOGE(PROTOCOL_TAG, "Failed to send packet 0x02: %s", esp_err_to_name(err));
        free(buf);
        return PROTOCOL_ERR_SEND;
    }
    ESP_LOGD(PROTOCOL_TAG, "Successfully sent packet 0x02 (%d bytes)", buf_size);
    free(buf);
    return ESP_OK;
}

int encode_packet_out_0x01(packet_out_0x01 packet, void** buf, int* len) {
    // Packet format: [4 bytes index][4 bytes length][variable data]
    *len = 4/*index*/ + 4/*len*/ + packet.len /*data*/;
    *buf = malloc(*len);
    if(*buf == NULL) {
        ESP_LOGE(PROTOCOL_TAG, "Failed to allocate buffer for packet encoding");
        return PROTOCOL_ERR_ALLOC;
    }
    memcpy((uint8_t*)*buf + 4 + 4, packet.data, packet.len);
    return 0;
}

//TODO: test
int send_packet_0x01(packet_out_0x01 packet) {
    void* buf = NULL;
    void* packet_ieee80211 = NULL;
    int len;
    
    int err = encode_packet_out_0x01(packet, &buf, &len);
    if(err != 0) {
        ESP_LOGE(PROTOCOL_TAG, "Failed to encode packet 0x01: %d", err);
        goto cleanup;
    }

    int packet_ieee80211_len = sizeof(ieee80211header) + sizeof(magic_number) + 1 /*packet id*/ + len;
    packet_ieee80211 = malloc(packet_ieee80211_len);
    if(packet_ieee80211 == NULL) {
        ESP_LOGE(PROTOCOL_TAG, "Failed to allocate %d bytes for IEEE802.11 packet", packet_ieee80211_len);
        err = PROTOCOL_ERR_ALLOC;
        goto cleanup;
    }

    // Build complete packet
    complete_packet_t packet = {
        .ieee80211_header = *(ieee80211_header_t*)ieee80211header,
        .protocol_header = {
            .magic = {magic_number[0], magic_number[1]},
            .id = 0x01,
            .crc = esp_crc32_le(PROTOCOL_CRC_INIT, buf, len)
        }
    };
    memcpy(&packet.payload.type_0x01, buf, len);
    
    // Copy to final buffer
    memcpy(packet_ieee80211, &packet, sizeof(ieee80211_header_t) + sizeof(protocol_header_t) + len);

    extern int esp_wifi_80211_tx_mod(wifi_interface_t ifx, const void *buffer, int len, bool en_sys_seq);

    err = esp_wifi_80211_tx_mod(WIFI_IF_STA, packet_ieee80211, packet_ieee80211_len, false);
    if(err != ESP_OK) {
        ESP_LOGE(PROTOCOL_TAG, "Failed to send packet 0x01: %s", esp_err_to_name(err));
        goto cleanup;
    }

    err = ESP_OK;

cleanup:
    if(buf) free(buf);
    if(packet_ieee80211) free(packet_ieee80211);
    return err;
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
    // Verify CRC
    uint32_t received_crc = header->crc;
    uint32_t calculated_crc = esp_crc32_le(PROTOCOL_CRC_INIT, 
                                         (uint8_t*)buffer + sizeof(header_t),
                                         length - sizeof(header_t));
                                         
    if(received_crc != calculated_crc) {
        ESP_LOGE(PROTOCOL_TAG, "CRC mismatch: received %08x, calculated %08x", 
                received_crc, calculated_crc);
        return PROTOCOL_ERR_CRC_MISMATCH;
    }

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
