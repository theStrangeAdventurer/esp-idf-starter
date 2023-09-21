
typedef void (*adc_callback_t)(int value_x, int value_y);  // Определение типа указателя на функцию колбэка

void init_adc(adc_callback_t callback);

bool check_is_center_position_x(int value_x);

bool check_is_center_position_y(int value_y);

