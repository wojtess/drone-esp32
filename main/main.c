#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <time.h>

#include "protocol.h"
#include "core.h"
#include "camera.h"

#include "esp_wifi.h"
#include "nvs_flash.h"
#include "esp_camera.h"

state_t state;

static size_t jpg_encode_stream(void * arg, size_t index, const void* data, size_t len) {
    printf("recived line\n");
    // int err = send_packet_0x01((packet_out_0x01){
    //     index,
    //     len,
    //     data
    // });
    // if(err != 0) {
    //     printf("error while sending packet0x01: %d\n", err);
    //     return -1;
    // }
    return 0;
}


void sniffer_callback(void* buf, wifi_promiscuous_pkt_type_t type) {
    // if(type != WIFI_PKT_DATA) {
    //     return;
    // }
    wifi_promiscuous_pkt_t* pkt = (wifi_promiscuous_pkt_t*)buf;
    int pkt_len = pkt->rx_ctrl.sig_len;

    void* buffer = pkt->payload;

    if(*(uint8_t*)buffer /*IEEE802.11 packet type and subtype*/ != 0x48) {
        //packet is wrong type
        return;
    }
    //IEEE header cut
    pkt_len -= 24;

    if(pkt_len < 3) {
        //packet to small to contain magic number and packet id
        return;
    }

    //IEEE header cut
    buffer += 24;

    header_t* header = (header_t*)buffer;

    if(header->magic[0] == 0x3C || header->magic[1] == 0x4A) {
        // printf("recived packet with matching magic number, length: %d, packet id: %d\n", pkt_len, header->id);
        buffer += sizeof(header_t);
        pkt_len -= sizeof(header_t);

        int err = decode_and_handle_packet(&state, header, buffer, pkt_len);
        if(err != 0) {
            printf("error while decoding and hadling packet: %d\n", err);
        }
    }
}

void sendTask(void* p) {
    // char packet_ieee80211[] = {
    //     0x48, 0x00, 0x00, 0x00,
    //     0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,//addr1
    //     0x13, 0x22, 0x33, 0x44, 0x55, 0x66,//addr2
    //     0x13, 0x22, 0x33, 0x44, 0x55, 0x66,//addr3
    //     0x00, 0x00,//duration or seq number, idk
    // };

    while(1) {
        // extern int esp_wifi_80211_tx_mod(wifi_interface_t ifx, const void *buffer, int len, bool en_sys_seq);

        // esp_wifi_80211_tx_mod(WIFI_IF_STA, packet_ieee80211, sizeof(packet_ieee80211), false);

        double throttle = 0.0;
        double yaw = 0.0;
        double pitch = 0.0;
        double roll = 0.0;

        uint64_t data[4];

        memcpy(data, &throttle, sizeof(throttle));
        memcpy(data + 1, &yaw, sizeof(yaw));
        memcpy(data + 2, &pitch, sizeof(pitch));
        memcpy(data + 3, &roll, sizeof(roll));

        packet_out_0x02 packet = {
            time(0),
            data[0],
            data[1],
            data[2],
            data[3]
        };

        send_packet_0x02(packet);

        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void capture_task(void* p) {
    camera_fb_t* fb;
    int quality = 80;
    void* arg = 0;

    int err = init_camera();
    if(err == ESP_OK) {
        printf("camera ok, starting capture\n");
    } else {
        printf("camera not ok, exiting from task\n");
        vTaskDelete(NULL);
    }

    bool camera_working = false;

    while(1) {
        fb = esp_camera_fb_get();

        if(fb != 0) {
            if(!camera_working) {
                printf("camera started working\n");
            }
            camera_working = true;
            frame2jpg_cb(fb, quality, &jpg_encode_stream, arg);
            esp_camera_fb_return(fb);
        } else {
            if(camera_working) {
                printf("camera stopped working\n");
            }
            camera_working = false;
        }

        //add delay if camera isnt working
        if(!camera_working) {
            vTaskDelay(500 / portTICK_PERIOD_MS);
        }
    }
}

void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();
    
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    const wifi_country_t wifi_country = {
			.cc = "CN",
			.schan = 1,
			.nchan = 13,
			.policy = WIFI_COUNTRY_POLICY_AUTO
	};
	ESP_ERROR_CHECK(esp_wifi_set_country(&wifi_country)); //set country for channel range [1, 13]

	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
	ESP_ERROR_CHECK(esp_wifi_start());

    ESP_ERROR_CHECK(esp_wifi_set_channel(11, 0));

    wifi_promiscuous_filter_t filter = {0};
    filter.filter_mask |= WIFI_PROMIS_FILTER_MASK_ALL;
    ESP_ERROR_CHECK(esp_wifi_set_promiscuous_filter(&filter));
    ESP_ERROR_CHECK(esp_wifi_set_promiscuous_rx_cb(sniffer_callback));
    ESP_ERROR_CHECK(esp_wifi_set_promiscuous(true));


    xTaskCreate(&capture_task, "captureTask", 4096, 0, 4, 0);
    xTaskCreate(&sendTask, "sendTask", 4096, 0, 5, 0);
}


// #include <esp_log.h>
// #include <esp_system.h>
// #include <nvs_flash.h>
// #include <sys/param.h>
// #include <string.h>

// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"

// // support IDF 5.x
// #ifndef portTICK_RATE_MS
// #define portTICK_RATE_MS portTICK_PERIOD_MS
// #endif

// #include "esp_camera.h"

// #define CAM_PIN_PWDN 32
// #define CAM_PIN_RESET -1 //software reset will be performed
// #define CAM_PIN_XCLK 0
// #define CAM_PIN_SIOD 26
// #define CAM_PIN_SIOC 27

// #define CAM_PIN_D7 35
// #define CAM_PIN_D6 34
// #define CAM_PIN_D5 39
// #define CAM_PIN_D4 36
// #define CAM_PIN_D3 21
// #define CAM_PIN_D2 19
// #define CAM_PIN_D1 18
// #define CAM_PIN_D0 5
// #define CAM_PIN_VSYNC 25
// #define CAM_PIN_HREF 23
// #define CAM_PIN_PCLK 22


// static const char *TAG = "example:take_picture";

// static camera_config_t camera_config = {
//     .pin_pwdn = CAM_PIN_PWDN,
//     .pin_reset = CAM_PIN_RESET,
//     .pin_xclk = CAM_PIN_XCLK,
//     .pin_sccb_sda = CAM_PIN_SIOD,
//     .pin_sccb_scl = CAM_PIN_SIOC,

//     .pin_d7 = CAM_PIN_D7,
//     .pin_d6 = CAM_PIN_D6,
//     .pin_d5 = CAM_PIN_D5,
//     .pin_d4 = CAM_PIN_D4,
//     .pin_d3 = CAM_PIN_D3,
//     .pin_d2 = CAM_PIN_D2,
//     .pin_d1 = CAM_PIN_D1,
//     .pin_d0 = CAM_PIN_D0,
//     .pin_vsync = CAM_PIN_VSYNC,
//     .pin_href = CAM_PIN_HREF,
//     .pin_pclk = CAM_PIN_PCLK,

//     //XCLK 20MHz or 10MHz for OV2640 double FPS (Experimental)
//     .xclk_freq_hz = 16000000,
//     .ledc_timer = LEDC_TIMER_0,
//     .ledc_channel = LEDC_CHANNEL_0,

//     .pixel_format = PIXFORMAT_JPEG, //YUV422,GRAYSCALE,RGB565,JPEG
//     .frame_size = FRAMESIZE_QVGA,    //QQVGA-UXGA, For ESP32, do not use sizes above QVGA when not JPEG. The performance of the ESP32-S series has improved a lot, but JPEG mode always gives better frame rates.

//     .jpeg_quality = 12, //0-63, for OV series camera sensors, lower number means higher quality
//     .fb_count = 1,       //When jpeg mode is used, if fb_count more than one, the driver will work in continuous mode.
//     .fb_location = CAMERA_FB_IN_PSRAM,
//     .grab_mode = CAMERA_GRAB_WHEN_EMPTY,
// };

// static esp_err_t init_camera(void)
// {
//     //initialize the camera
//     esp_err_t err = esp_camera_init(&camera_config);
//     if (err != ESP_OK)
//     {
//         ESP_LOGE(TAG, "Camera Init Failed");
//         return err;
//     }

//     return ESP_OK;
// }

// void app_main(void)
// {
//     if(ESP_OK != init_camera()) {
//         return;
//     }

//     while (1)
//     {
//         ESP_LOGI(TAG, "Taking picture...");
//         camera_fb_t *pic = esp_camera_fb_get();

//         // use pic->buf to access the image
//         ESP_LOGI(TAG, "Picture taken! Its size was: %zu bytes", pic->len);
//         esp_camera_fb_return(pic);

//         vTaskDelay(5000 / portTICK_RATE_MS);
//     }
// }