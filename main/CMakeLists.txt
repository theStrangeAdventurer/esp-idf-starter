idf_component_register(
    SRCS "main.c" "led/rgb_led.c" "server.c" "wifi.c" "fs.c" "lcd1602/i2c-lcd.c"
    INCLUDE_DIRS "." "led" "lcd1602"
)

set(WEB_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../frontend-app")
if(EXISTS ${WEB_SRC_DIR}/dist)
    spiffs_create_partition_image(www ${WEB_SRC_DIR}/dist FLASH_IN_PROJECT)
else()
    message(FATAL_ERROR "${WEB_SRC_DIR}/dist doesn't exit. Please run 'npm run build' in ${WEB_SRC_DIR}")
endif()