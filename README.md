# Levoit Humidifier Integration for ESPHome
Modifications to make a Levoit OasisMist Humidifier use ESPHome local control. The stock PCB has an ESP32-SOLO-1C with test-points for GND, VDD, RX, TX, EN, & IO0 for programing. All of the user interface and control seems to be implemented on the SC92F84A3M MCU similar to other Levoit devices.

## Supported Devices
* LHU-O451S-WUSR - Levoit OasisMist Smart Humidifier (Confirmed)

## Disassembly
* In progress...

## Flash
* Copy the provided sample configuration for your model to a new ESPHome configuration, fill out wifi, encryption, and password for your setup.
* Compile and download the binary (Choose the modern format once compilation has completed).
* Solder wires to test-points TXD, RXD, IO0, VDD, and GND near the ESP32 on the logic board, and connect these to a USB UART converter.
* Connect IO0 to ground while applying power to boot into the bootloader.

### Backup Existing Firmware
```bash
esptool read-flash 0 ALL levoit-humidifier-stock_firmware-$(date +"%Y%m%d").bin
```

### Erase Flash
```bash
esptool erase-flash
```

### Install New Firmware
```bash
esptool write-flash 0x00 levoit-humidifier-firmware.bin
```

Disconnect and reconnect the USB UART to make sure the ESP32 restarts. Once powered, check to make sure the ESP32 connects to your wireless network.

After confirming, reassemble, and enjoy :)

# Contributing
All contributions are welcome! Please open an issue or a PR.

## Contributors

## Special Thanks
* Aiden (@acvigue) - [ESPHome Levoit Air Purifier](https://github.com/acvigue/esphome-levoit-air-purifier) used as a base for this project.
