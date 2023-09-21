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
#include <stdio.h>
#include <sys/param.h>
#include <esp_http_server.h>
#include "esp_tls.h"
#include <esp_system.h>
#include "utils.h"

/* LCD 1602 >> */
#include "i2c-lcd.h"
#include "driver/i2c.h"
/* << LCD 1602 */

/* ADC read (Joystick) >> */
#include "adc-read.h"
/* << ADC read (Joystick) */

/* GPIO >> */
#include "driver/gpio.h"
#include "rgb_led.h"
/* << GPIO */

#define HTTP_QUERY_KEY_MAX_LEN  (64)

#define LED GPIO_NUM_2

static const char *TAG = "app_Main"; // Tag for logger

static int client_connected = 0;

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

static void write_lcd_message()
{
    lcd_clear();
    lcd_put_cur(0, 0);

    if (client_connected) {
        lcd_send_string("Connected!");
        lcd_put_cur(1, 0);
        lcd_send_string("IP: 192.168.4.1");
    } else {
        lcd_send_string("Awaiting ");
        lcd_put_cur(1, 0);
        lcd_send_string("connection...");
    }
}

/* HTTP Server handlers >> */
static void disconnect_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data)
{
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server) {
        ESP_LOGI(TAG, "Stopping webserver");
        if (stop_webserver(*server) == ESP_OK) {
            *server = NULL;
            client_connected = 0;
            led_off();
            write_lcd_message();
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
        client_connected = 1;
        write_lcd_message();
        led_on();
    }
}
/* << HTTP Server handlers */

static int last_lcd_message_type = 0; // 0 - Сообщение о статусе подключения. 1 - Сообщение о координатах

/* ADC Handler */
static void adc_callback(int x, int y)
{
    if (check_is_center_position_x(x) && check_is_center_position_y(y)) {
        if (!last_lcd_message_type) {
            write_lcd_message();
            rgb_led_set_color(255, 255, 255);
            last_lcd_message_type = 1;
        }
        return;
    }
    last_lcd_message_type = 0;

    lcd_clear();
    lcd_put_cur(0, 0);
    lcd_send_string("Stick:");
    lcd_put_cur(1, 0);

    char message[100];

    // Переводим значение в uint_8 (0-255)
    int mapped_x = map_value(x, 512, 255);
    int mapped_y = map_value(y, 512, 255);

    snprintf(
        message,
        sizeof(message),
        "x=%d, y=%d",
        mapped_x,
        mapped_y
    );
    rgb_led_set_color(125, mapped_x, mapped_y);
    lcd_send_string(message);
    ESP_LOGI(TAG, "%s", message);
}
/* ADC Handler */

void app_main(void)
{
    static httpd_handle_t server = NULL;

    configure_led();
    
    // Initialize LCD display
    ESP_ERROR_CHECK(i2c_master_init());
    ESP_LOGI(TAG, "I2C initialized successfully");
    lcd_init();
    write_lcd_message();
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

    init_adc(adc_callback);
    rgb_led_set_color(255, 255, 255);

    /*
     * Starting web server when user connected to wi-fi
     */
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_AP_STACONNECTED, &connect_handler, &server));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_AP_STADISCONNECTED, &disconnect_handler, &server));
}
