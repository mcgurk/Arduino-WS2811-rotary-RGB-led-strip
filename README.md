# Arduino-WS2811-rotary-RGB-led-strip

## LedStrip_v2.ino

### Hardware

- Arduino Pro Micro (Arduino Leonardo selected from Arduino IDE)
- LED strings: "DC 5V 50PCS WS2811 RGB Full Color 12mm Pixels digital Addressable LED String XD"
- Databus to pin2
- 150pcs WS2811 leds with full brightness and rainbow-colors takes 5V/3A
- 150pcs WS2811 leds with 1/2 brightness and rainbow-colors takes 5V/2A
- ATmega32u4 SRAM: 2560B (~600 RGB leds)
- ATmega328p SRAM: 2048B (~? RGB leds)

### Libraries
- NeoPixelBus by Makuna (https://github.com/Makuna/NeoPixelBus/wiki)
- Encoder (https://www.pjrc.com/teensy/td_libs_Encoder.html)
- (MemoryUsage (https://github.com/Locoduino/MemoryUsage))

Fast HSV -> RGB taken from here: http://www.vagrearg.org/content/hsvrgb

Can't use assembler version:
```
error: r28 cannot be used in asm here
error: r29 cannot be used in asm here
```
Google says that works with optimization level 1, but I don't know easy way to change optimization level.

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

Back to rainbow mode:
```
ser.write(b'r')
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
#imgarr[:,:,1] = 0 # G = 0
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


## WS2811_valo.ino

### Usage
- Rotate to change brightness.
- Click and after that rotate to change color/temperature.
- 1s click saves parameters.
- 2s click goes to rainbowmode, where you can change brightness and after click speed.

![Image](https://github.com/mcgurk/Arduino-WS2811-rotary-RGB-led-strip/raw/master/Arduino_UNO_WS2811-ledstrip_with_rotaryencoder.jpg)




