/*
 * SPDX-FileCopyrightText: 2022-2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <sys/param.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_vfs_fat.h"
#include "esp_heap_caps.h"
#include "driver/spi_master.h"
#include "driver/sdmmc_host.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "sdkconfig.h"
#include "quirc.h"
#include "quirc_internal.h"
#include "esp_camera.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "nvs_flash.h"

static const char *TAG = "example";

// Camera image size for QR code detection - optimized for speed
#define IMG_WIDTH 320
#define IMG_HEIGHT 240
#define CAM_FRAME_SIZE FRAMESIZE_QVGA

// Camera configuration for Freenove WROOM board
// Updated with the specific pin configuration provided
#define CAM_PIN_PWDN    -1 //power down is not used
#define CAM_PIN_RESET   -1 //software reset will be performed
#define CAM_PIN_XCLK    15
#define CAM_PIN_SIOD    4
#define CAM_PIN_SIOC    5

#define CAM_PIN_D7      16
#define CAM_PIN_D6      17
#define CAM_PIN_D5      18
#define CAM_PIN_D4      12
#define CAM_PIN_D3      10
#define CAM_PIN_D2      8
#define CAM_PIN_D1      9
#define CAM_PIN_D0      11
#define CAM_PIN_VSYNC   6
#define CAM_PIN_HREF    7
#define CAM_PIN_PCLK    13

static uint8_t rgb565_to_grayscale(const uint8_t *img);
static void processing_task(void *arg);
static void main_task(void *arg);
static esp_err_t init_camera(void);
static esp_err_t init_wifi(void);
static esp_err_t connect_to_wifi(const char *ssid, const char *password);
static bool parse_wifi_qr_code(const char *qr_data, char *ssid, char *password);
static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);

// WiFi connection status
static bool wifi_connected = false;
static char connected_ssid[64] = {0};
static bool camera_stopped = false;  // Flag to indicate camera has been stopped

// Application entry point
void app_main(void)
{
    xTaskCreatePinnedToCore(&main_task, "main", 4096, NULL, 5, NULL, 0);
}

// Initialize camera with error handling
static esp_err_t init_camera(void)
{
    ESP_LOGI(TAG, "Initializing camera...");
    
    // Initialize the camera
    camera_config_t camera_config = {
        .pin_pwdn = CAM_PIN_PWDN,
        .pin_reset = CAM_PIN_RESET,
        .pin_xclk = CAM_PIN_XCLK,
        .pin_sscb_sda = CAM_PIN_SIOD,
        .pin_sscb_scl = CAM_PIN_SIOC,

        .pin_d7 = CAM_PIN_D7,
        .pin_d6 = CAM_PIN_D6,
        .pin_d5 = CAM_PIN_D5,
        .pin_d4 = CAM_PIN_D4,
        .pin_d3 = CAM_PIN_D3,
        .pin_d2 = CAM_PIN_D2,
        .pin_d1 = CAM_PIN_D1,
        .pin_d0 = CAM_PIN_D0,
        .pin_vsync = CAM_PIN_VSYNC,
        .pin_href = CAM_PIN_HREF,
        .pin_pclk = CAM_PIN_PCLK,

        .xclk_freq_hz = 20000000,
        .ledc_channel = LEDC_CHANNEL_0,
        .ledc_timer = LEDC_TIMER_0,
        .pixel_format = PIXFORMAT_GRAYSCALE,  // Changed to grayscale for faster processing
        .frame_size = FRAMESIZE_QVGA,         // Reduced to 320x240 for speed
        .jpeg_quality = 5,                    // Lowest quality for speed
        .fb_count = 1,                        // Single frame buffer
        .grab_mode = CAMERA_GRAB_WHEN_EMPTY,
    };

    esp_err_t err = esp_camera_init(&camera_config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Camera init failed with error 0x%x: %s", err, esp_err_to_name(err));
        
        // Try with different I2C pins if the first attempt fails
        ESP_LOGI(TAG, "Trying alternative I2C pins...");
        camera_config.pin_sccb_sda = 21;  // Alternative SDA pin
        camera_config.pin_sccb_scl = 22;  // Alternative SCL pin
        
        err = esp_camera_init(&camera_config);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Camera init failed with alternative pins, error 0x%x: %s", err, esp_err_to_name(err));
            return err;
        }
        ESP_LOGI(TAG, "Camera initialized with alternative I2C pins");
    } else {
        ESP_LOGI(TAG, "Camera initialized successfully");
    }

    sensor_t *s = esp_camera_sensor_get();
    if (s) {
        s->set_vflip(s, 1);
        ESP_LOGI(TAG, "Camera sensor configured");
    } else {
        ESP_LOGW(TAG, "Could not get camera sensor");
    }

    // Disable the LED to prevent flashing
    ledc_stop(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0);
    ESP_LOGI(TAG, "LED disabled to prevent flashing");

    return ESP_OK;
}

// Initialize WiFi
static esp_err_t init_wifi(void)
{
    ESP_LOGI(TAG, "Initializing WiFi...");
    
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize TCP/IP stack
    ESP_ERROR_CHECK(esp_netif_init());
    
    // Create default event loop
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    
    // Create default netif instance
    esp_netif_create_default_wifi_sta();

    // Initialize WiFi
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // Register event handlers
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL));

    // Set WiFi mode to station
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    
    // Start WiFi
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "WiFi initialized successfully");
    return ESP_OK;
}

// WiFi event handler
static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        ESP_LOGI(TAG, "WiFi station started");
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGI(TAG, "WiFi disconnected, trying to reconnect...");
        wifi_connected = false;
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
        wifi_connected = true;
    }
}

// Connect to WiFi network
static esp_err_t connect_to_wifi(const char *ssid, const char *password)
{
    ESP_LOGI(TAG, "Connecting to WiFi: %s", ssid);
    
    wifi_config_t wifi_config = {
        .sta = {
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            .pmf_cfg = {
                .capable = true,
                .required = false
            },
        },
    };
    
    // Copy SSID and password
    strncpy((char*)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid) - 1);
    strncpy((char*)wifi_config.sta.password, password, sizeof(wifi_config.sta.password) - 1);
    
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_connect());
    
    return ESP_OK;
}

// Parse WiFi QR code format: S:SSID;P:Password
static bool parse_wifi_qr_code(const char *qr_data, char *ssid, char *password)
{
    ESP_LOGI(TAG, "Parsing QR code: %s", qr_data);
    
    // Look for S: and P: patterns
    const char *ssid_start = strstr(qr_data, "S:");
    const char *pass_start = strstr(qr_data, "P:");
    
    if (!ssid_start || !pass_start) {
        ESP_LOGW(TAG, "Invalid WiFi QR code format");
        return false;
    }
    
    // Extract SSID (skip "S:")
    ssid_start += 2;
    const char *ssid_end = strchr(ssid_start, ';');
    if (!ssid_end) {
        ESP_LOGW(TAG, "SSID not properly terminated");
        return false;
    }
    
    int ssid_len = ssid_end - ssid_start;
    if (ssid_len >= 64) ssid_len = 63;
    strncpy(ssid, ssid_start, ssid_len);
    ssid[ssid_len] = '\0';
    
    // Extract password (skip "P:")
    pass_start += 2;
    const char *pass_end = strchr(pass_start, ';');
    if (!pass_end) {
        // Password might be at the end without semicolon
        pass_end = pass_start + strlen(pass_start);
    }
    
    int pass_len = pass_end - pass_start;
    if (pass_len >= 64) pass_len = 63;
    strncpy(password, pass_start, pass_len);
    password[pass_len] = '\0';
    
    ESP_LOGI(TAG, "Parsed SSID: %s, Password: %s", ssid, password);
    return true;
}

// Main task: initializes the camera and starts the processing task
static void main_task(void *arg)
{
    ESP_LOGI(TAG, "Starting QR Code Demo for Freenove WROOM Board");

    // Initialize WiFi
    esp_err_t wifi_err = init_wifi();
    if (wifi_err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize WiFi");
    }

    // Initialize the camera with error handling
    esp_err_t err = init_camera();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize camera. Please check:");
        ESP_LOGE(TAG, "1. Camera module connections");
        ESP_LOGE(TAG, "2. Power supply (3.3V)");
        ESP_LOGE(TAG, "3. I2C pull-up resistors");
        ESP_LOGE(TAG, "4. Pin assignments");
        ESP_LOGE(TAG, "Continuing without camera...");
        
        // Continue without camera for testing
        while (1) {
            ESP_LOGI(TAG, "Camera not available - demo cannot run");
            vTaskDelay(pdMS_TO_TICKS(5000));
        }
    }

    // The queue for passing camera frames to the processing task
    QueueHandle_t processing_queue = xQueueCreate(1, sizeof(camera_fb_t *));
    assert(processing_queue);

    // The processing task will be running QR code detection and recognition
    xTaskCreatePinnedToCore(&processing_task, "processing", 35000, processing_queue, 1, NULL, 0);
    ESP_LOGI(TAG, "Processing task started");

    // Main loop: capture frames and send them to the processing task
    while (1) {
        // Check if camera has been stopped
        if (camera_stopped) {
            ESP_LOGI(TAG, "Main task stopping - camera deinitialized");
            break;
        }
        
        camera_fb_t *fb = esp_camera_fb_get();
        if (!fb) {
            ESP_LOGE(TAG, "Camera capture failed");
            vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }

        // Send the frame to the processing task
        if (xQueueSend(processing_queue, &fb, pdMS_TO_TICKS(100)) != pdTRUE) {
            ESP_LOGW(TAG, "Queue full, dropping frame");
            esp_camera_fb_return(fb);
        }

        vTaskDelay(pdMS_TO_TICKS(20)); // 50 FPS for faster scanning
    }
    
    // Clean exit
    ESP_LOGI(TAG, "Main task completed successfully");
    vTaskDelete(NULL);
}

// Processing task: receives camera frames and performs QR code detection
static void processing_task(void *arg)
{
    QueueHandle_t processing_queue = (QueueHandle_t)arg;
    struct quirc *qr = quirc_new();
    assert(qr);

    if (quirc_resize(qr, IMG_WIDTH, IMG_HEIGHT) < 0) {
        ESP_LOGE(TAG, "Failed to allocate QR code buffer");
        vTaskDelete(NULL);
    }

    ESP_LOGI(TAG, "QR code detection initialized");

    while (1) {
        camera_fb_t *fb;
        if (xQueueReceive(processing_queue, &fb, portMAX_DELAY) != pdTRUE) {
            continue;
        }

        uint8_t *image = quirc_begin(qr, NULL, NULL);
        // Since we're using PIXFORMAT_GRAYSCALE, we can directly copy the data
        memcpy(image, fb->buf, fb->len);
        quirc_end(qr);

        int count = quirc_count(qr);
        ESP_LOGI(TAG, "QR count: %d   Heap: %d  Stack free: %d  time: %d ms", 
                 count, esp_get_free_heap_size(), uxTaskGetStackHighWaterMark(NULL), 
                 esp_timer_get_time() / 1000);

        for (int i = 0; i < count; i++) {
            struct quirc_code code = {};
            struct quirc_data qr_data = {};
            quirc_extract(qr, i, &code);
            quirc_decode_error_t err = quirc_decode(&code, &qr_data);

            if (err) {
                ESP_LOGW(TAG, "Decoding failed: %s", quirc_strerror(err));
            } else {
                ESP_LOGI(TAG, "Decoded in %d ms", esp_timer_get_time() / 1000);
                ESP_LOGI(TAG, "QR code: %d bytes: '%s'", qr_data.payload_len, qr_data.payload);
                
                // Check if this is a WiFi QR code
                char ssid[64] = {0};
                char password[64] = {0};
                
                if (parse_wifi_qr_code((const char*)qr_data.payload, ssid, password)) {
                    ESP_LOGI(TAG, "WiFi QR code detected! Attempting to connect...");
                    
                    // Connect to WiFi
                    esp_err_t connect_err = connect_to_wifi(ssid, password);
                    if (connect_err == ESP_OK) {
                        ESP_LOGI(TAG, "WiFi connection initiated for: %s", ssid);
                        
                        // Wait for connection (up to 10 seconds)
                        int attempts = 0;
                        while (!wifi_connected && attempts < 100) {
                            vTaskDelay(pdMS_TO_TICKS(100));
                            attempts++;
                        }
                        
                        if (wifi_connected) {
                            strncpy(connected_ssid, ssid, sizeof(connected_ssid) - 1);
                            ESP_LOGI(TAG, "Connected to %s WiFi!", connected_ssid);
                            ESP_LOGI(TAG, "QR code scanning stopped.");
                            
                            // Stop camera and exit all tasks
                            esp_camera_deinit();
                            ESP_LOGI(TAG, "Camera deinitialized");
                            camera_stopped = true;  // Set flag to stop main task
                            
                            // Exit both tasks cleanly
                            vTaskDelete(NULL);
                            return;
                        } else {
                            ESP_LOGE(TAG, "Failed to connect to WiFi after 10 seconds");
                        }
                    } else {
                        ESP_LOGE(TAG, "Failed to initiate WiFi connection");
                    }
                }
            }
        }

        esp_camera_fb_return(fb);
    }
}

// Color conversion utilities
typedef union {
    uint16_t val;
    struct {
        uint16_t b: 5;
        uint16_t g: 6;
        uint16_t r: 5;
    };
} rgb565_t;

static uint8_t rgb565_to_grayscale(const uint8_t *img)
{
    rgb565_t *rgb = (rgb565_t *)img;
    return (rgb->r * 299 + rgb->g * 587 + rgb->b * 114) / 1000;
}
