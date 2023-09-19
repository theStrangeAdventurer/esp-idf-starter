/*  WiFi softAP Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include "sdkconfig.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include <stdlib.h>
#include <unistd.h>
#include <sys/param.h>
#include <esp_http_server.h>
#include "esp_tls.h"
#include <esp_system.h>

/* GPIO >> */
#include "driver/gpio.h"
#include "rgb_led.h"
/* << GPIO */

/* FS >>*/
#include "esp_spiffs.h"
#include "esp_vfs.h"
#include <dirent.h>
/* << FS*/

#define HTTP_QUERY_KEY_MAX_LEN  (64)

#define LED GPIO_NUM_2

static const char *TAG = "app"; // Tag for logger

/* Definitions for server.c file >> */
httpd_handle_t start_webserver(const char *base_path);
esp_err_t stop_webserver(httpd_handle_t server);
/* << Definitions for server.c file */

/* Definitions for wifi.c file >> */
void wifi_init_softap(void);
/* << Definitions for wifi.c file */

/* configure FS */
esp_err_t init_fs(void)
{
    esp_vfs_spiffs_conf_t conf = {
        .base_path = CONFIG_WEB_MOUNT_POINT,
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = false
    };
    esp_err_t ret = esp_vfs_spiffs_register(&conf);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        } else {
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return ESP_FAIL;
    }

    size_t total = 0, used = 0;
    ret = esp_spiffs_info(NULL, &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
    } else {
        ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
    }
    return ESP_OK;
}
/* configure FS */

/* configure LED pin */
void configure_led() {
    gpio_reset_pin(LED);
    gpio_set_direction(LED, GPIO_MODE_OUTPUT);
}

void led_on() {
    ESP_LOGI(TAG, "led state: ON");
    gpio_set_level(LED, 1);
}

void led_off() {
    ESP_LOGI(TAG, "led state: OFF");
    gpio_set_level(LED, 0);
}

/* configure LED pin */

/**
 * HTTP Server handlers
 */
static void disconnect_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data)
{
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server) {
        ESP_LOGI(TAG, "Stopping webserver");
        if (stop_webserver(*server) == ESP_OK) {
            *server = NULL;
            led_off();
        } else {
            ESP_LOGE(TAG, "Failed to stop http server");
        }
    }
}

static void connect_handler(void* arg, esp_event_base_t event_base,
                            int32_t event_id, void* event_data)
{
    ESP_LOGI(TAG, "connect_handler start");
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server == NULL) {
        ESP_LOGI(TAG, "Starting webserver");
        *server = start_webserver(CONFIG_WEB_MOUNT_POINT);
        led_on();
        rgb_led_wifi_connected();
    }
}

void app_main(void)
{
    static httpd_handle_t server = NULL;

    configure_led();
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    ESP_LOGI(TAG, "ESP_WIFI_MODE_AP");
    wifi_init_softap();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(init_fs());

    /*
     * Starting web server when user connected to wi-fi
     */
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_AP_STACONNECTED, &connect_handler, &server));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_AP_STADISCONNECTED, &disconnect_handler, &server));
}
