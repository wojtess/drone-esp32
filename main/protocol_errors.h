#ifndef PROTOCOL_ERRORS_H
#define PROTOCOL_ERRORS_H

#include "esp_err.h"
#include "esp_log.h"

#define PROTOCOL_TAG "PROTOCOL"

typedef enum {
    PROTOCOL_OK = 0,
    PROTOCOL_ERR_ALLOC = -1,
    PROTOCOL_ERR_INVALID_PACKET = -2,
    PROTOCOL_ERR_SEND = -3,
    PROTOCOL_ERR_INVALID_ARG = -4,
    PROTOCOL_ERR_DECODE = -5
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
