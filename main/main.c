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

/* LCD 1602 >> */
#include "i2c-lcd.h"
#include "driver/i2c.h"
/* << LCD 1602 */

/* GPIO >> */
#include "driver/gpio.h"
#include "rgb_led.h"
/* << GPIO */

#define HTTP_QUERY_KEY_MAX_LEN  (64)

#define LED GPIO_NUM_2

static const char *TAG = "app_Main"; // Tag for logger

/* Definitions for server.c file >> */
httpd_handle_t start_webserver(const char *base_path);
esp_err_t stop_webserver(httpd_handle_t server);
/* << Definitions for server.c file */

/* Definitions for wifi.c file >> */
void wifi_init_softap(void);
/* << Definitions for wifi.c file */

/* Definitions for fs.c file >> */
esp_err_t init_fs(void);
/* << Definitions for fs.c file */


/* Configure i2c LCD 1602 */
/**
 * @brief i2c master initialization
 */
static esp_err_t i2c_master_init(void)
{
    int i2c_master_port = I2C_NUM_0;

    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = GPIO_NUM_21,
        .scl_io_num = GPIO_NUM_22,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 100000,
    };

    i2c_param_config(i2c_master_port, &conf);

    return i2c_driver_install(i2c_master_port, conf.mode, 0, 0, 0);
}

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

/* HTTP Server handlers >> */
static void disconnect_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data)
{
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server) {
        ESP_LOGI(TAG, "Stopping webserver");
        if (stop_webserver(*server) == ESP_OK) {
            *server = NULL;
            led_off();
            lcd_clear();
            lcd_put_cur(0, 0);
            lcd_send_string("Awaiting ");
            lcd_put_cur(1, 0);
            lcd_send_string("connection...");
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

        lcd_put_cur(0, 0);
        lcd_send_string("Connected!");

        lcd_put_cur(1, 0);
        lcd_send_string("IP: 192.168.4.1");

        led_on();
        rgb_led_wifi_connected();
    }
}
/* << HTTP Server handlers */

void app_main(void)
{
    static httpd_handle_t server = NULL;

    configure_led();
    
    // Initialize LCD display
    ESP_ERROR_CHECK(i2c_master_init());
    ESP_LOGI(TAG, "I2C initialized successfully");
    lcd_init();
    lcd_clear();
    lcd_put_cur(0, 0);
    lcd_send_string("Awaiting");
    lcd_put_cur(1, 0);
    lcd_send_string("connection...");

    // Initialize NVS
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
