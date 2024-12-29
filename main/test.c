#include <stdio.h>
#include <assert.h>
#include "protocol.h"
#include "core.h"

void test_packet_handling() {
    printf("Testing packet handling...\n");
    
    // Initialize test state
    state_t state = {0};
    
    // Test packet 0x01
    packet_in_0x01 pkt1 = {
        .throttle = end_htobe64(0x3FF0000000000000), // Double representation of 1.0
        .pitch = end_htobe64(0x3FE0000000000000),    // Double representation of 0.5
        .roll = end_htobe64(0xBFE0000000000000),     // Double representation of -0.5
        .yaw = end_htobe64(0x3FD0000000000000)       // Double representation of 0.25
    };
    
    protocol_header_t hdr1 = {
        .magic = {magic_number[0], magic_number[1]},
        .id = 0x01,
        .crc = esp_crc32_le(PROTOCOL_CRC_INIT, (uint8_t*)&pkt1, sizeof(pkt1))
    };
    
    int result = decode_and_handle_packet(&state, &hdr1, &pkt1, sizeof(pkt1));
    assert(result == 0);
    assert(state.controls.throttle == 1.0);
    assert(state.controls.pitch == 0.5);
    assert(state.controls.roll == -0.5);
    assert(state.controls.yaw == 0.25);
    
    printf("Packet 0x01 tests passed!\n");

    // Test packet 0x02
    packet_in_0x02 pkt2 = {
        .frequency = 1000,
        .packet_3 = true
    };
    
    protocol_header_t hdr2 = {
        .magic = {magic_number[0], magic_number[1]},
        .id = 0x02,
        .crc = esp_crc32_le(PROTOCOL_CRC_INIT, (uint8_t*)&pkt2, sizeof(pkt2))
    };
    
    result = decode_and_handle_packet(&state, &hdr2, &pkt2, sizeof(pkt2));
    assert(result == 0);
    assert(state.pwmControls.freq == 1000);
    assert(state.pwmRaw == true);
    
    printf("Packet 0x02 tests passed!\n");

    // Test packet 0x03
    packet_in_0x03 pkt3 = {
        .duty0 = 100,
        .duty1 = 200,
        .duty2 = 300,
        .duty3 = 400
    };
    
    protocol_header_t hdr3 = {
        .magic = {magic_number[0], magic_number[1]},
        .id = 0x03,
        .crc = esp_crc32_le(PROTOCOL_CRC_INIT, (uint8_t*)&pkt3, sizeof(pkt3))
    };
    
    result = decode_and_handle_packet(&state, &hdr3, &pkt3, sizeof(pkt3));
    assert(result == 0);
    assert(state.pwmControls.duty0 == 100);
    assert(state.pwmControls.duty1 == 200);
    assert(state.pwmControls.duty2 == 300);
    assert(state.pwmControls.duty3 == 400);
    
    printf("Packet 0x03 tests passed!\n");
}

int main() {
    test_packet_handling();
    printf("All tests passed!\n");
    return 0;
}
