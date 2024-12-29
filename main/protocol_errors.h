#ifndef PROTOCOL_ERRORS_H
#define PROTOCOL_ERRORS_H

#include "esp_err.h"
#include "esp_log.h"

#define PROTOCOL_TAG "PROTOCOL"

/**
 * @brief Protocol error codes
 * 
 * These codes are used to indicate specific error conditions in the protocol layer.
 * Negative values indicate errors, 0 indicates success.
 */
typedef enum {
    PROTOCOL_OK = 0,                ///< Operation completed successfully
    PROTOCOL_ERR_ALLOC = -1,        ///< Memory allocation failed
    PROTOCOL_ERR_INVALID_PACKET = -2, ///< Received packet is invalid or malformed
    PROTOCOL_ERR_SEND = -3,         ///< Failed to send packet
    PROTOCOL_ERR_INVALID_ARG = -4,  ///< Invalid argument passed to function
    PROTOCOL_ERR_DECODE = -5,       ///< Failed to decode packet
    PROTOCOL_ERR_CRC_MISMATCH = -6  ///< CRC check failed (data corruption)
} protocol_err_t;

#define PROTOCOL_CHECK(x) do { \
        esp_err_t __err_rc = (x); \
        if (__err_rc != ESP_OK) { \
            ESP_LOGE(PROTOCOL_TAG, "%s:%d (%s): %s", __FILE__, __LINE__, __FUNCTION__, esp_err_to_name(__err_rc)); \
            return __err_rc; \
        } \
    } while(0)

#define PROTOCOL_CHECK_ARG(x) do { \
        if (!(x)) { \
            ESP_LOGE(PROTOCOL_TAG, "%s:%d (%s): Invalid argument", __FILE__, __LINE__, __FUNCTION__); \
            return PROTOCOL_ERR_INVALID_ARG; \
        } \
    } while(0)

#endif // PROTOCOL_ERRORS_H
