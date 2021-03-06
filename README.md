# Arduino-WS2811-rotary-RGB-led-strip

## LedStrip_v2.ino

### Hardware

- Arduino Pro Micro (Arduino Leonardo selected from Arduino IDE)
- LED strings: "DC 5V 50PCS WS2811 RGB Full Color 12mm Pixels digital Addressable LED String XD"
- Databus to pin2
- 150pcs WS2811 leds with 255 brightness and rainbow-colors takes 5V/3A
- 150pcs WS2811 leds with 128 brightness and rainbow-colors takes 5V/2A
- brightness 200 -> 2.7A, brightness 175 -> 2.5A
- ATmega32u4 SRAM: 2560B (~600 RGB leds)
- ATmega328p SRAM: 2048B (~? RGB leds)
- If ArduinoJson or other libraries which needs buffers or uses lot of memory, max. 300 RGB leds could be better choice.

### Libraries
- NeoPixelBus 2.3.4 by Makuna (https://github.com/Makuna/NeoPixelBus/wiki)
- ArduinoJson 5.13.2 (6.x is beta)
- MemoryUsage (https://github.com/Locoduino/MemoryUsage)
- (Encoder (https://www.pjrc.com/teensy/td_libs_Encoder.html))

Fast HSV -> RGB taken from here: http://www.vagrearg.org/content/hsvrgb

### Arduino IDE, fast_hsv2rgb, optimization level

Can't use assembler version with fast hsv2rgb:
```
error: r28 cannot be used in asm here
error: r29 cannot be used in asm here
```
Google says that works with optimization level 1, but I don't know easy way to change optimization level.

Found:
```
C:\Program Files (x86)\Arduino\hardware\arduino\avr\platform.txt
compiler.cpp.flags: -OS -> -O1
```
https://docs.oracle.com/cd/E37670_01/E52461/html/ch04s03.html

Can't still get it work :(. Returns allways only zeros.

### Python

#### Prepare python, if needed

Orange Pi PC, Armbian 5.60 (jessie) 3.4.113-sun8i
```
sudo apt install python3-dev
sudo python3 -m pip install pillow
sudo python3 -m pip install pyserial
sudo python3 -m pip install numpy
```
Orange Pi Zero, Armbian 5.60 (Stretch) 4.14.70-sunxi
```
sudo apt install python3-pip python3-dev python3-setuptools zlib1g-dev libjpeg-dev
sudo python3 -m pip install wheel
sudo python3 -m pip install pillow
sudo python3 -m pip install pyserial
sudo python3 -m pip install numpy
```
Raspberry Pi - Raspbian: included in Raspbian Desktop

Program which loads 150x300 image and sends one line at the time to led string:
```
from PIL import Image
import numpy as np
import serial
import time

ser = serial.Serial('/dev/ttyACM0', 2000000)
img = Image.open("/misc/valo.png").convert("RGB")
imgarr = np.array(img)

def play(cnt):
  for i in range(cnt):
    line = i % 300
    ser.write(imgarr[line].flatten())
    time.sleep(0.02)

play(1000)
```
This gives about 35fps.

Command examples (JSON):
```
import serial
ser = serial.Serial('/dev/ttyACM0', 2000000)
ser.write(b'{"pixels":300, "mode":"rainbow", "brightness":128, "fading":"false", "speed":2, "periods":2}')
ser.close()

import serial
ser = serial.Serial('/dev/ttyACM0', 2000000)
ser.write(b'{"pixels":300, "mode":"rainbow", "brightness":5, "fading":"false", "speed":10, "periods":10}')
ser.close()

{"pixels":300, "mode":"rainbow", "brightness":128, "fading":"false", "speed":2, "periods":2, "save":"true"}
# (pixels, mode, brightness can be saved to eeprom with {"save":"true"})

# fill
{"mode":"fill", "r":10, "g":10, "b":10}
```

Reset Arduino Pro Micro with Python:
```
import serial
ser = serial.Serial('/dev/ttyACM0', 1200)
ser.close()
```

Reset Arduino Pro Micro from shell:
```
stty -F /dev/ttyACM0 1200
```

Minicom:
```
minicom -D /dev/ttyACM0 -b 2000000
```

Flash from shell (sudo apt install avrdude):
```
stty -F /dev/ttyACM0 1200 ; sleep 1 ; avrdude -p m32u4 -b 57600 -P /dev/ttyACM0 -c avr109 -U flash:w:/misc/LedStrip_v2.ino.leonardo.hex:i
```

Clear
```
import numpy as np
import serial
ser = serial.Serial('/dev/ttyACM0', 2000000)
# data = np.zeros((600,3), dtype=np.uint8)
data = np.full((600, 3), [10,10,10], dtype=np.uint8)
ser.write(data)
```

strip.py (150 wide image, strip is 300 wide, so double image. strip is not grb, neopixelbus pixelorder doesn't work?)
```
#!/usr/bin/python3

from PIL import Image
import numpy as np
import serial
import time

ser = serial.Serial('/dev/ttyACM0', 2000000)
img = Image.open("/misc/valo.png").convert("RGB")
imgarr = np.array(img)
#imgarr[:,:,1] = 0 # G = 0 (R0, G1, B2)
imgarr[:,:,[1,0]] = imgarr[:,:,[0,1]] # RGB -> GRB

i = 0
while 1:
  line = i % 600
  data = imgarr[line] // 2
  ser.write(data)
  ser.write(data)
  time.sleep(0.02)
  i += 1
```

## Node-RED
E.g. Slider-node -> Template-node -> MQTT-node:
```
{"{{topic}}":"{{payload}}"}
```
Colorwheel works out of box.

## WS2811_valo.ino

### Usage
- Rotate to change brightness.
- Click and after that rotate to change color/temperature.
- 1s click saves parameters.
- 2s click goes to rainbowmode, where you can change brightness and after click speed.

![Image](https://github.com/mcgurk/Arduino-WS2811-rotary-RGB-led-strip/raw/master/Arduino_UNO_WS2811-ledstrip_with_rotaryencoder.jpg)




