# Wireless Control Protocol Specification

## Overview
This protocol is designed for low-latency wireless control of devices using ESP32's raw 802.11 packet transmission. Since ESP32 can only send WiFi packets, we use IEEE 802.11 raw packet transmission with NULL function to implement our custom protocol. It includes error detection via CRC32 checksums and supports multiple packet types.

## Packet Structure
All packets follow this format:

```
[IEEE 802.11 Header][Protocol Header][Payload]
```

### IEEE 802.11 Header (24 bytes)
The IEEE 802.11 header is required for raw packet transmission. We use NULL function (type 0x48) which doesn't require association with an access point. The header structure is:

```
0x48, 0x00, 0x00, 0x00,          // Frame Control + Duration
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // Destination MAC (broadcast)
0x13, 0x22, 0x33, 0x44, 0x55, 0x66, // Source MAC
0x13, 0x22, 0x33, 0x44, 0x55, 0x66, // BSSID
0x00, 0x00                        // Sequence Control
```

### Protocol Header (7 bytes)
Our custom protocol header follows the IEEE 802.11 header:

```
[Magic Number][Packet ID][CRC32]
```

Fields:
   
1. **Magic Number** (2 bytes)
   - Fixed value 0x3C4A to identify our protocol packets
   - Helps distinguish our packets from other WiFi traffic
   
2. **Packet ID** (1 byte)
   - Identifies the packet type:
     - 0x01: Control packet (throttle, pitch, roll, yaw)
     - 0x02: Configuration packet
     - 0x03: PWM duty cycle packet
     
3. **CRC32** (4 bytes)
   - CRC32 checksum of entire packet (header + payload)
   - Uses ESP32 hardware acceleration
   - Initial value: 0xFFFFFFFF
   - Protects against transmission errors

### Payload (variable)
   - Packet-specific data (see packet types below)

### Complete Packet Example
Here's how a complete packet looks in memory:

```
[IEEE 802.11 Header (24 bytes)]
48 00 00 00 FF FF FF FF FF FF 13 22 33 44 55 66 13 22 33 44 55 66 00 00

[Protocol Header (7 bytes)]
3C 4A 01 12 34 56 78

[Payload (variable)]
... packet specific data ...
```

The CRC32 in this example would be calculated over both the protocol header and payload.

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
