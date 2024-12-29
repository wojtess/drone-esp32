# Wireless Control Protocol Specification

## Overview
This protocol is designed for low-latency wireless control of devices using ESP32's raw 802.11 packet transmission. It includes error detection via CRC32 checksums and supports multiple packet types.

## Packet Structure
All packets follow this format:

```
[IEEE 802.11 Header][Magic Number][Packet ID][CRC32][Payload]
```

### Fields:
1. **IEEE 802.11 Header** (24 bytes)
   - Standard 802.11 header for raw packet transmission
   
2. **Magic Number** (2 bytes)
   - Fixed value 0x3C4A to identify our protocol packets
   
3. **Packet ID** (1 byte)
   - Identifies the packet type:
     - 0x01: Control packet (throttle, pitch, roll, yaw)
     - 0x02: Configuration packet
     - 0x03: PWM duty cycle packet
     
4. **CRC32** (4 bytes)
   - CRC32 checksum of entire packet (header + payload)
   - Uses ESP32 hardware acceleration
   - Initial value: 0xFFFFFFFF
   
5. **Payload** (variable)
   - Packet-specific data (see packet types below)

## Packet Types

### 0x01 - Control Packet
Contains control inputs for the device:
```c
struct {
    uint64_t throttle; // Big-endian
    uint64_t pitch;    // Big-endian
    uint64_t roll;     // Big-endian
    uint64_t yaw;      // Big-endian
}
```

### 0x02 - Configuration Packet
Contains device configuration:
```c
struct {
    uint32_t frequency; // PWM frequency
    bool use_raw_pwm;   // Use raw PWM values (0x03) instead of control inputs
}
```

### 0x03 - PWM Duty Cycle Packet
Contains raw PWM values:
```c
struct {
    uint32_t duty0; // Motor 0 duty cycle
    uint32_t duty1; // Motor 1 duty cycle
    uint32_t duty2; // Motor 2 duty cycle
    uint32_t duty3; // Motor 3 duty cycle
}
```

## Error Detection
Each packet includes a CRC32 checksum calculated over the entire packet (header + payload). Receivers should:
1. Calculate CRC32 of received packet
2. Compare with CRC32 in packet header
3. Discard packet if CRC32 doesn't match

## Endianness
All multi-byte values are transmitted in big-endian (network) byte order. Receivers must convert to host byte order using provided conversion functions.

## Reliability Considerations
- No built-in retransmission mechanism
- No sequence numbers for packet ordering
- Designed for low-latency rather than reliability
- Applications should handle packet loss appropriately
