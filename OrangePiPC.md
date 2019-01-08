- Armbian_5.65_Orangepipc_Debian_stretch_next_4.14.78.7z
- Linux orangepipc 4.14.84-sunxi #3 SMP Sat Dec 1 07:18:41 CET 2018 armv7l GNU/Linux

- http://www.orangepi.org/orangepibbsen//forum.php?mod=viewthread&tid=3318&page=1&extra=#pid21903
- http://www.electrobob.com/ws2812-level-translator/
- https://wp.josh.com/2014/05/13/ws2812-neopixels-are-not-so-finicky-once-you-get-to-know-them/
- https://hackaday.com/2017/01/20/cheating-at-5v-ws2812-control-to-use-a-3-3v-data-line/

/boot/armbianEnv.txt:
```
overlays=spi-spidev
param_spidev_spi_bus=0
param_spidev_max_freq=100000000
```

- https://i.stack.imgur.com/lzt4s.png
- SPI0 MOSI = Pin10, GPIO 19 (PC0) (data output from master)

```
/etc/udev/rules.d/50-spi.rules:
SUBSYSTEM=="spidev", GROUP="spiuser", MODE="0660"

sudo udevadm control --reload-rules
sudo groupadd spiuser
sudo adduser "$USER" spiuser
(muista että groupadd ei tuu voimaan ennen uutta loggausta)

sudo modprobe -r spidev
sudo modprobe spidev
```


```
sudo apt install python3-pip python3-setuptools python3-dev python3-wheel python3-numpy
sudo pip3 install spidev
git clone https://github.com/joosteto/ws2812-spi.git
cd ws2812-spi
```

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
sudo python3 setup.py install
```

Test:
```
import spidev
import ws2812
spi = spidev.SpiDev()
spi.open(0,0)
ws2812.write2812(spi, [[10,0,0], [0,10,0], [0,0,10], [10, 10, 0]])
```
## Audio
- https://www.swharden.com/wp/2016-07-19-realtime-audio-visualization-in-python/
- https://www.programcreek.com/python/example/52624/pyaudio.PyAudio
```
sudo apt install portaudio19-dev
sudo pip3 install pyaudio
```

```
import pyaudio
import numpy as np
import spidev
import ws2812
spi = spidev.SpiDev()
spi.open(0,0)
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
