#ifndef MAIN_RGB_LED_H_
#define MAIN_RGB_LED_H_

// RGB led configuration
typedef struct
{
    int channel;
    int gpio;
    int mode;
    int timer_index;
} rgb_ledc_t;

void rgb_led_set_color(uint8_t red, uint8_t green, uint8_t blue);

#endif /* MAIN_RGB_LED_H_ */