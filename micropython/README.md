https://docs.micropython.org/en/v1.8/esp8266/esp8266/tutorial/neopixel.html

## Schematics
- Wemos D1 (ESP8266, 4MB)
- 24 WS2812 RGB led ring
- 5V -> VCC
- D2 (GPIO4) -> Data
- GND -> GND
- D5 (GPIO14) -> Button (input, pullup)
- D6 (GPIO12) -> Button ("gnd")

## hsv_to_rgb
https://stackoverflow.com/questions/24852345/hsv-to-rgb-color-conversion
- input scale: 1.0, 1.0, 1.0 (float)
- output scale: 255, 255, 255 (int)
```
def hsv_to_rgb(h, s, v):
  if s == 0.0: v*=255; return (v, v, v)
  i = int(h*6.) # XXX assume int() truncates!
  f = (h*6.)-i; p,q,t = int(255*(v*(1.-s))), int(255*(v*(1.-s*f))), int(255*(v*(1.-s*(1.-f)))); v*=255; i%=6
  v = int(v)
  if i == 0: return (v, t, p)
  if i == 1: return (q, v, p)
  if i == 2: return (p, v, t)
  if i == 3: return (p, q, v)
  if i == 4: return (t, p, v)
  if i == 5: return (v, p, q)
```

## Flashing
- http://micropython.org/download
- http://micropython.org/resources/firmware/esp8266-20180511-v1.9.4.bin
- http://docs.micropython.org/en/latest/index.html
- https://docs.micropython.org/en/latest/reference/constrained.html

did't get to work anything else than esptool.py (Linux/Windows):
```
sudo pip3 install esptool
esptool.py erase_flash
esptool.py --baud 460800 write_flash -fm dio 0 esp8266-20180511-v1.9.4.bin
```
(in WSL only 115200baud works)

## Pastemode
```
ctrl-E
code=r'''(paste)'''
ctrl-D
```
then save that variable into the target file with 
```
open('target.py','w') as f: f.write(code)
```

## "Soft reset"
`Ctrl-d`
Cleans memory, loads boot.py and main.py

## Windows subsystem for Linux / rshell:
- https://github.com/dhylands/rshell
- https://github.com/Microsoft/WSL/issues/3668
```
sudo adduser "$USER" dialout
sudo apt update
(sudo apt install screen minicom)
(screen /dev/ttyS6 115200)
(minicom -D /dev/ttyS6)
sudo apt install python3 python3-pip
sudo pip3 install rshell
don't work:
rshell -p /dev/ttyS6
rshell -p /dev/ttyS6 cp main.py /pyboard
in rshell this works:
connect serial /dev/ttyS6
```
- Hint: `Ctrl-x` leaves program running but exists from REPL.
- If you copy-paste bigger chunks, remember that in rshell there is limited buffer.
