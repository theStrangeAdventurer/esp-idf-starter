| Supported Targets | ESP32 | ESP32-C2 | ESP32-C3 | ESP32-C6 | ESP32-S2 | ESP32-S3 |
| ----------------- | ----- | -------- | -------- | -------- | -------- | -------- |

### Build and Flash

> Assuming you have an alias `get_idf='. $HOME/esp/esp-idf/export.sh'` in your shell, it sets the environment variables in the working directory.
> [Manual installation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/linux-macos-setup.html#step-2-get-esp-idf)

0. `idf.py menuconfig`, then *Serial flasher config* > *Flash size*, set `4mb`
1. `idf.py menuconfig`, then change *PARTITION_TABLE_FILENAME* and *PARTITION_TABLE_CUSTOM_FILENAME* to `partitions_webapp.csv`
2. `vi $IDF_PATH/components/partition_table/partitions_webapp.csv` and paste content from `partitions_webapp.csv`
3. You have to change `idf.port` in `.vscode/settings.json` if you're using Espressif VScode extension. You can determine actual port by using command `ls -l /dev/tty.*` (MacOS, Linux)


Build the project and flash it to the board, then run the monitor tool to view the serial output:

Run `idf.py -p PORT flash monitor` to build, flash and monitor the project.

(To exit the serial monitor, type ``Ctrl-]``.)

See the Getting Started Guide for all the steps to configure and use the ESP-IDF to build projects.

* [ESP-IDF Getting Started Guide on ESP32](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html)
* [ESP-IDF Getting Started Guide on ESP32-S2](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s2/get-started/index.html)
* [ESP-IDF Getting Started Guide on ESP32-C3](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c3/get-started/index.html)

## Troubleshooting

0. `partitions_webapp.csv not found in ....`. Please Run command `vi $IDF_PATH/components/partition_table/partitions_webapp.csv` and paste content from partitions_webapp.csv
1. If you are using the cmake tools extension in VScode and receive the error *"xtensa-esp32-elf-gcc is not a full path and was not found in the PATH"* while running CMake:Configure command, you need to add the *xtensa-esp32-elf-gcc* binary file path to your env `PATH`. You can determine the location of the binary using the `where xtensa-esp32-elf-gcc` command after you've used the `get_idf` alias
