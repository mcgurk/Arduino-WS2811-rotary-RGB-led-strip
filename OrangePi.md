## Orange Pi

### Tested
- Armbian_5.75_Orangepipc_Debian_stretch_next_4.19.20.7z / Armbian_5.75_Orangepizero_Debian_stretch_next_4.19.20.7z
- Linux orangepipc 4.19.20-sunxi #5.75 SMP Sat Feb 9 19:02:47 CET 2019 armv7l GNU/Linux

### Preparation/settings
```
sudo armbian-config
```
- Timezone
- Wifi
- (sudo apt install avahi-daemon avahi-discover libnss-mdns)

### Links
- http://www.orangepi.org/orangepibbsen//forum.php?mod=viewthread&tid=3318&page=1&extra=#pid21903
- http://www.electrobob.com/ws2812-level-translator/
- https://wp.josh.com/2014/05/13/ws2812-neopixels-are-not-so-finicky-once-you-get-to-know-them/
- https://hackaday.com/2017/01/20/cheating-at-5v-ws2812-control-to-use-a-3-3v-data-line/
- Level shifter comparison with scope: https://happyinmotion.com/?p=1247

### Wiring
- https://github.com/joosteto/ws2812-spi
- Orange Pi PC: https://i.stack.imgur.com/lzt4s.png
- Orange Pi Zero: https://i.stack.imgur.com/O03j0.jpg
- Orange Pi PC: SPI0 MOSI = 10 (GPIO10/PC0) (physical pin 19) (data output from master)
- Orange Pi Zero: SPI1 MOSI = 15 (GPIO15/MOSI) (physical pin 19) (data output from master)

### Enable SPI
/boot/armbianEnv.txt:
```
overlay_prefix=sun8i-h3
overlays=spi-spidev
param_spidev_spi_bus=0
param_spidev_max_freq=100000000
```
/etc/udev/rules.d/50-spi.rules:
```
SUBSYSTEM=="spidev", GROUP="spiuser", MODE="0660"
```
```
sudo udevadm control --reload-rules
sudo groupadd spiuser
sudo adduser "$USER" spiuser
```
(remember that groupadd is not effective before new login)

Reload module if neened:
```
sudo modprobe -r spidev
sudo modprobe spidev
```

### Python / ws2812-spi -library
```
sudo apt install python3-pip python3-setuptools python3-dev python3-wheel python3-numpy
sudo pip3 install spidev
git clone https://github.com/mcgurk/ws2812-spi.git
cd ws2812-spi
```

#### Test
Orange Pi PC:
```
python3 ws2812.py -t
```
Orange Pi Zero:
```
python3 ws2812.py -t -s 1
```

#### Install
```
sudo python3 setup.py install
```

#### Simple test example
```
import spidev
import ws2812
spi = spidev.SpiDev()
spi.open(0,0) # If Zero: spi.open(1,0)
ws2812.write2812(spi, [[10,0,0], [0,10,0], [0,0,10], [10, 10, 0]])
```

## Audio
- https://www.swharden.com/wp/2016-07-19-realtime-audio-visualization-in-python/
- https://www.programcreek.com/python/example/52624/pyaudio.PyAudio

Set input volume and gain (settings are saved):
```
amixer set Mic1 cap
amixer set 'Mic1 Boost' 100
amixer set 'ADC Gain' 100
```

Test:
```
arecord -vv /dev/null
```

```
sudo apt install python3-pyaudio
#sudo apt install portaudio19-dev
#sudo pip3 install pyaudio
```

```
import pyaudio
import numpy as np
import spidev
import ws2812
spi = spidev.SpiDev()
spi.open(0,0) # If Zero: spi.open(1,0)
ws2812.write2812(spi, [[10,0,0], [0,10,0], [0,0,10], [10, 10, 0]])

CHUNK = 2**11
RATE = 44100

p=pyaudio.PyAudio()
stream=p.open(format=pyaudio.paInt16,channels=1,rate=RATE,input=True,
              frames_per_buffer=CHUNK)

out = np.zeros((50,3), dtype=int)
while True:
#for i in range(int(10*44100/1024)): #go for a few seconds
    data = np.fromstring(stream.read(CHUNK),dtype=np.int16)
    #peak=np.average(np.abs(data))*2
    #bars="#"*int(50*peak/2**16)
    peak = np.amax(np.abs(data))
    bars = "#" * int(peak/200)
    #print("%04d %05d %s"%(i,peak,bars))
    #print("%05d %s"%(peak,bars))
    out.fill(0)
    out[0:int(peak/200)] = (100, 0, 0)
    #out[0:5] = (100, 0, 0)
    ws2812.write2812(spi, out)

stream.stop_stream()
stream.close()
p.terminate()
```

### Patch for https://github.com/joosteto/ws2812-spi.git
patch.txt:
```
--- ws2812-spi/ws2812.py        2019-01-08 12:18:06.102975277 +0200
+++ ws2812-spi_oma/ws2812.py    2019-01-08 10:33:14.162554845 +0200
@@ -38,7 +38,10 @@ def write2812_numpy4(spi,data):
         tx[3-ibit::4]=((d>>(2*ibit+1))&1)*0x60 + ((d>>(2*ibit+0))&1)*0x06 +  0x88
         #print [hex(v) for v in tx]
     #print [hex(v) for v in tx]
-    spi.xfer(tx.tolist(), int(4/1.25e-6)) #works, on Zero (initially didn't?)
+    spi.max_speed_hz = int(4/1.05e-6)
+    spi.writebytes(tx.tolist())
+    #spi.xfer(tx.tolist(), int(4/1.05e-6)) #works, on Zero (initially didn't?)
+    #spi.xfer(tx.tolist(), int(4/1.25e-6)) #works, on Zero (initially didn't?)
     #spi.xfer(tx.tolist(), int(4/1.20e-6))  #works, no flashes on Zero, Works on Raspberry 3
     #spi.xfer(tx.tolist(), int(4/1.15e-6))  #works, no flashes on Zero
     #spi.xfer(tx.tolist(), int(4/1.05e-6))  #works, no flashes on Zero
@@ -102,7 +105,7 @@ if __name__=="__main__":
         opts, args = getopt.getopt(sys.argv[1:], "hn:c:t", ["help", "color=", "test"])
     except getopt.GetoptError as err:
         # print help information and exit:
-        print str(err) # will print something like "option -a not recognized"
+        print(str(err)) # will print something like "option -a not recognized"
         usage()
         sys.exit(2)
     color=None
```

```
patch < patch.txt
sudo python3 setup.py install
```

## Rainbow
```
# (Orange Pi PC, sleep 0.01, 50led: ~20% 1.3GHz, 300led: ~40% 1.3GHz)

import numpy as np
import signal, sys, time
import spidev, ws2812, colorsys
spi = spidev.SpiDev()
spi.open(0,0) # If Zero: spi.open(1,0)

pixels = 50

def signal_handler(sig, frame):
        print('You pressed Ctrl+C!')
        data = np.zeros((pixels,3), dtype=np.int8)
        ws2812.write2812(spi, data)
        sys.exit(0)

signal.signal(signal.SIGINT, signal_handler)

gamma_table = (0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,3,3,3,3,3,3,3,4,4,4,4,4,5,5,5,5,6,6,6,6,7,7,7,7,8,8,8,9,9,9,10,10,10,11,11,11,12,12,13,13,13,14,14,15,15,16,16,17,17,18,18,19,19,20,20,21,21,22,22,23,24,24,25,25,26,27,27,28,29,29,30,31,32,32,33,34,35,35,36,37,38,39,39,40,41,42,43,44,45,46,47,48,49,50,50,51,52,54,55,56,57,58,59,60,61,62,63,64,66,67,68,69,70,72,73,74,75,77,78,79,81,82,83,85,86,87,89,90,92,93,95,96,98,99,101,102,104,105,107,109,110,112,114,115,117,119,120,122,124,126,127,129,131,133,135,137,138,140,142,144,146,148,150,152,154,156,158,160,162,164,167,169,171,173,175,177,180,182,184,186,189,191,193,196,198,200,203,205,208,210,213,215,218,220,223,225,228,231,233,236,239,241,244,247,249,252,255)

def gamma(c):
  r = max(min(int(c[0]), 255), 0)
  g = max(min(int(c[1]), 255), 0)
  b = max(min(int(c[2]), 255), 0)
  return (r, g, b)

data = np.zeros((pixels,3), dtype=np.int8)
while True:
  t = time.time() / 5.0
  for i in range(pixels):
    h = (i/pixels + t) % 1.0
    data[i] = gamma(colorsys.hsv_to_rgb(h, 1.0, 200))
  ws2812.write2812(spi, data)
  time.sleep(0.01)
```

## SPI buffer size
- Default maximum data size is 4096 bytes (300 leds -> ok, 350 -> doesn't work).
- Increase https://www.raspberrypi.org/forums/viewtopic.php?t=124472

Orange Pi (not tested):
```
modprobe spidev bufsiz=32768
check:
cat /sys/module/spidev/parameters/bufsiz
Hardcoded?
"linux/spidev.c"  the line 91:"static unsigned bufsiz = 4096;"
```
Python (not tested):
```
1. download the py-spidev module as a zip file and extract the contents;
2. edit the 'spidev_module.c' file. There are four instances of '4096' in the file. Use search and replace to change all four of these to the desired value. Save the file.
3. remove the existing spidev module:
sudo pip uninstall spidev
4. reinstall spidev using the modified source file:
sudo python setup.py install
```
