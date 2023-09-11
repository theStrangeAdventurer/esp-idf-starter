| Supported Targets | ESP32 | ESP32-C2 | ESP32-C3 | ESP32-C6 | ESP32-S2 | ESP32-S3 |
| ----------------- | ----- | -------- | -------- | -------- | -------- | -------- |

### Build and Flash

0. `idf.py menuconfig`, then *Serial flasher config* > *Flash size*, set `4mb`
1. `idf.py menuconfig`, then change *PARTITION_TABLE_FILENAME* and *PARTITION_TABLE_CUSTOM_FILENAME* to `partitions_webapp.csv`
2. `vi $IDF_PATH/components/partition_table/partitions_webapp.csv` and paste content from `partitions_webapp.csv`

Build the project and flash it to the board, then run the monitor tool to view the serial output:

Run `idf.py -p PORT flash monitor` to build, flash and monitor the project.

(To exit the serial monitor, type ``Ctrl-]``.)

See the Getting Started Guide for all the steps to configure and use the ESP-IDF to build projects.

* [ESP-IDF Getting Started Guide on ESP32](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html)
* [ESP-IDF Getting Started Guide on ESP32-S2](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s2/get-started/index.html)
* [ESP-IDF Getting Started Guide on ESP32-C3](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c3/get-started/index.html)

## Troubleshooting

0. `partitions_webapp.csv not found in ....`. Please Run command `vi $IDF_PATH/components/partition_table/partitions_webapp.csv` and paste content from partitions_webapp.csv
