#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"
#include "adc-read.h"

// В качестве ADC используется джойстик, x и y провода которого подключены к
// GPIO 34, 35
#define ADC_CHANNEL_X ADC_CHANNEL_6
#define ADC_CHANNEL_Y ADC_CHANNEL_7
#define ADC_CALLBACK_STACK_SIZE 4096
#define ADC_TASK_PRIORITY 5

adc_callback_t adc_callback = NULL;

int previous_x = 241; // Значения, которые чаще всего показывает джойстик на середине
int previous_y = 248; // Значения, которые чаще всего показывает джойстик на середине

bool check_is_center_position_x(int x)
{
    return (x > 235) && (x < 250); // Значения в середине джойстика колеблются в этих значениях
}

bool check_is_center_position_y(int y)
{
    return (y > 240) && (y < 253); // Значения в середине джойстика колеблются в этих значениях
}

static void async_adc_read_callback(int x, int y)
{
    if ((x != previous_x) || (y != previous_y)) {
        if (adc_callback != NULL) {
            adc_callback(x, y);  // Вызов колбэка для x, если значение изменилось
        }
        // Обновление предыдущего значения
        previous_x = x;  
        previous_y = y;
    }
}

static void async_adc_read_task(void *params) {
    adc_oneshot_unit_handle_t adc1_handle;
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));
    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_9, // 0 to 512 values
        .atten = ADC_ATTEN_DB_11, // The only one value which get valid values
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_X, &config));
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_Y, &config));

    while (1) {
        int x = 0;
        int y = 0;
        adc_oneshot_read(adc1_handle, ADC_CHANNEL_X, &x);
        adc_oneshot_read(adc1_handle, ADC_CHANNEL_Y, &y);
        async_adc_read_callback(x, y);  // Вызываем обработчик колбэка после каждого чтения

        vTaskDelay(pdMS_TO_TICKS(100));  // Пауза между опросами ADC
    }
}

void init_adc(adc_callback_t callback) {
    adc_callback = callback;  // Устанавливаем указатель на функцию колбэка
    // Инициализация ADC и запуск асинхронного опроса
    xTaskCreate(
        async_adc_read_task,
        "adc_read_task",
        ADC_CALLBACK_STACK_SIZE,
        NULL,
        ADC_TASK_PRIORITY,
        NULL
    );
}

