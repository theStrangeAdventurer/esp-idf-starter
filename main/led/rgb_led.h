#ifndef MAIN_RGB_LED_H_
#define MAIN_RGB_LED_H_

// RGB led GPIOs 
#define RGB_LED_RED_GPIO    21
#define RGB_LED_GREEN_GPIO  22
#define RGB_LED_BLUE_GPIO   23

// RGB led color mix channels
#define RGB_LED_CHANNEL_NUM 3

// RGB led configuration
typedef struct
{
    int channel;
    int gpio;
    int mode;
    int timer_index;
} rgb_ledc_t;

void rgb_led_wifi_connected(void);

#endif /* MAIN_RGB_LED_H_ */