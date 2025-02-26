Use with **idf v5.4** \
Based on https://gist.github.com/wojtess/ec441a1efa374b8b02f5e52633bfa3f4

### What is working
- Camera - dont work
- Reciving data from ground station - **works**
- Controlling PWM based on throttle, yaw, pitch and roll - **works** *not tested*

### Protocol

Protocol uses raw ieee802.11 frames to communicate. It uses NULL frame with additional data after original frame, it breaks ieee802.11 specification, beacuse of that normal devices should reject all packets used by this protocol.

## Packet Structure

All packets share a common header followed by packet-specific data:

- **802.11 Header**: Standard 802.11 header (24 bytes).
- **Magic Number**: `0x3C 0x4A` (2 bytes). Identifies the packet as belonging to this protocol.
- **Packet ID**: 1 byte. Indicates the type of data contained in the packet.
- **Payload**: Variable length. The content and format of the payload depend on the Packet ID.
- **CRC32 Checksum**: 4 bytes. Calculated over the entire packet after the 802.11 header but before the CRC itself. Ensures data integrity.


## Packet Definitions


### Outgoing Packets (Drone to Ground Station)

#### Packet 0x01: Image Data (OUT)

Transmits a chunk of image data from the drone's camera.

**Payload Structure:**

- **Index**: 4 bytes (uint32_t). Represents the starting index of the data chunk within the image.  *Not currently used by the receiver.*
- **Length**: 4 bytes (uint32_t). Indicates the length of the data chunk in bytes. *Not currently used by the receiver.*
- **Data**: `Length` bytes. Contains the raw image data.


**Example C Structure:**

```c
typedef struct {
    uint32_t index;
    uint32_t len;
    const void* data;
} packet_out_0x01;
```


#### Packet 0x02: Telemetry Data (OUT)

Sends telemetry data from the drone, including control inputs.

**Payload Structure:**

- **Timestamp**: 8 bytes (uint64_t). Unix timestamp.
- **Throttle**: 8 bytes (double, big-endian). Throttle control value.
- **Yaw**: 8 bytes (double, big-endian). Yaw control value.
- **Pitch**: 8 bytes (double, big-endian). Pitch control value.
- **Roll**: 8 bytes (double, big-endian). Roll control value.

**Example C Structure:**

```c
typedef struct {
    uint64_t timestamp;
    double throttle;
    double yaw;
    double pitch;
    double roll;
} packet_out_0x02;
```



### Incoming Packets (Ground Station to Drone)

#### Packet 0x01: Control Commands (IN)

Sends control commands to the drone.

**Payload Structure:**

- **Throttle**: 8 bytes (double, big-endian).  Throttle command value.
- **Pitch**: 8 bytes (double, big-endian). Pitch command value.
- **Roll**: 8 bytes (double, big-endian). Roll command value.
- **Yaw**: 8 bytes (double, big-endian). Yaw command value.

**Example C Structure:**

```c
typedef struct {
    double throttle;
    double pitch;
    double roll;
    double yaw; 
} packet_in_0x01;
```


#### Packet 0x02: Config (IN)

Sets the PWM frequency for motor control.

**Payload Structure:**

- **pwmRaw**: If true then using data from packet_0x03, if false use data from packet_0x01.
- **frequency**: Frequency of PWM signal for BLCD controller

**Example C Structure:**

```c
typedef struct {
    bool pwmRaw;
    uint32_t frequency;
} packet_in_0x02;
```

#### Packet 0x03: PWM Duty Cycle Control (IN)

Sets the PWM duty cycle for each motor, pwmRaw in packet_in_0x02 need to be set to true.

**Payload Structure:**

- **duty0**: Duty cycle for motor1
- **duty1**: Duty cycle for motor2
- **duty2**: Duty cycle for motor3
- **duty3**: Duty cycle for motor4

**Example C Structure:**

```c
typedef struct {
    uint32_t duty0;
    uint32_t duty1;
    uint32_t duty2;
    uint32_t duty3;
} packet_in_0x03;
```


## CRC32 Calculation

CRC32 is calculated using the standard polynomial `0xEDB88320U`. The implementation is provided in the `calculate_crc32` function in `main.c`.

## Implementation Notes

- The code uses the ESP-IDF framework.
- The `esp_wifi_80211_tx_mod` function is used for sending raw 802.11 frames.

