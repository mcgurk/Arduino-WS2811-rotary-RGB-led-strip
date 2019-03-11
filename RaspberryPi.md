- Enable SPI from Raspberry config
- If RPi 3, /boot/config.txt: core_freq=250
- https://github.com/jgarff/rpi_ws281x

```
sudo apt install scons
git clone https://github.com/jgarff/rpi_ws281x.git
cd rpi_ws281x
scons
```

SPI (GPIO10, physical pin 19):
```
./test -g 10 -x 24 -y 1 
```
PCM:
```
./test -g 21
```

```
cd python
sudo apt install swig
sudo python3 setup.py install
cd examples
nano strandtest.py # make sure that LED_PIN 10 (physical pin 19)
python3 strandtest.py
```

```
#PYTHON TEST
import time
from neopixel import *

LED_COUNT      = 16      # Number of LED pixels.
LED_BRIGHTNESS = 255     # Set to 0 for darkest and 255 for brightest

strip = Adafruit_NeoPixel(LED_COUNT, 10, 800000, 10, False, LED_BRIGHTNESS, 0)
strip.begin()

for i in range(strip.numPixels()):
  strip.setPixelColor(i, Color(10,i,0))

strip.show()
#PYTHON TEST END
```
```
#PYTHON RPi RAINBOW
import time
from neopixel import *
import colorsys

LED_COUNT      = 150      # Number of LED pixels.
LED_BRIGHTNESS = 255      # Set to 0 for darkest and 255 for brightest
PERIODS = 2
VELOCITY = 6

strip = Adafruit_NeoPixel(LED_COUNT, 10, 800000, 10, False, LED_BRIGHTNESS, 0)
strip.begin()

gamma_table = (0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,3,3,3,3,3,3,3,4,4,4,4,4,5,5,5,5,6,6,6,6,7,7,7,7,8,8,8,9,9,9,10,10,10,11,11,11,12,12,13,13,13,14,14,15,15,16,16,17,17,18,18,19,19,20,20,21,21,22,22,23,24,24,25,25,26,27,27,28,29,29,30,31,32,32,33,34,35,35,36,37,38,39,39,40,41,42,43,44,45,46,47,48,49,50,50,51,52,54,55,56,57,58,59,60,61,62,63,64,66,67,68,69,70,72,73,74,75,77,78,79,81,82,83,85,86,87,89,90,92,93,95,96,98,99,101,102,104,105,107,109,110,112,114,115,117,119,120,122,124,126,127,129,131,133,135,137,138,140,142,144,146,148,150,152,154,156,158,160,162,164,167,169,171,173,175,177,180,182,184,186,189,191,193,196,198,200,203,205,208,210,213,215,218,220,223,225,228,231,233,236,239,241,244,247,249,252,255)

def gamma(c):
  r = max(min(int(c[0]), 255), 0)
  g = max(min(int(c[1]), 255), 0)
  b = max(min(int(c[2]), 255), 0)
  return (Color(r, g, b))

while True:
  t = time.time() * VELOCITY / 100
  #t = VELOCITY / (time.time() % 50)
  for i in range(LED_COUNT):
    h = ((i*PERIODS)/LED_COUNT + t) % 1.0
    c = gamma(colorsys.hsv_to_rgb(h, 1, BRIGHTNESS))
    strip.setPixelColor(i, c)
  strip.show()
  time.sleep(0.01)
#END
```
