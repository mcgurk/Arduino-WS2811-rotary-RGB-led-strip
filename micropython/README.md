## Schematics
- Wemos D1 (ESP8266, 4MB)
- 24 WS2812 RGB led ring
- 5V -> VCC
- D2 (GPIO4) -> Data
- GND -> GND
- D5 (GPIO14) -> Button (input, pullup)
- D6 (GPIO12) -> Button (gnd)

## Flashing
- http://micropython.org/download
- http://micropython.org/resources/firmware/esp8266-20180511-v1.9.4.bin
- http://docs.micropython.org/en/latest/index.html
- https://docs.micropython.org/en/latest/reference/constrained.html

did't get to work anything else than esptool.py (Linux/Windows)
```
pip3 install esptool
esptool.py erase_flash
esptool.py --baud 460800 write_flash -fm dio 0 esp8266-20180511-v1.9.4.bin
```

## Windows subsystem for Linux:
```
sudo adduser "$USER" dialout
sudo apt update
sudo apt install screen minicom
screen /dev/ttyS6 115200
minicom -D /dev/ttyS6
sudo apt install python3 python3-pip
https://github.com/dhylands/rshell
sudo pip3 install rshell
don't work:
rshell -p /dev/ttyS6
in rshell this works:
connect serial /dev/ttyS6
(https://github.com/Microsoft/WSL/issues/3668)
```

