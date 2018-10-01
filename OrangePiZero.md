# Orange Pi Zero, Armbian 5.60 (Stretch) 4.14.70-sunxi

## SPI-device

There is no /dev/spidev* -devices at start. We can't use /dev/spidev0.0, because it's used by flash-memory. Let's make /dev/spidev1.0:
/boot/armbianEnv.txt:
```
overlays=spi-spidev usbhost2 usbhost3
param_spidev_spi_bus=1
param_spidev_max_freq=100000000
```
Reboot

`crw------- 1 root root 153, 0 Oct  1 10:45 /dev/spidev1.0`

Let's make it accessible without root. 
/etc/udev/rules.d/50-spi.rules:
`SUBSYSTEM=="spidev", GROUP="spiuser", MODE="0660"`
```
sudo udevadm control --reload-rules
sudo groupadd spiuser
sudo adduser "$USER" spiuser
```
(remember that adding to group doesn't come active until new login)

```
sudo modprobe -r spidev
sudo modprobe spidev
```

`crw-rw---- 1 root spiuser 153, 0 Oct  1 10:56 /dev/spidev1.0`
 
## Python 3

```
sudo apt install python3-pip python3-dev python3-setuptools zlib1g-dev libjpeg-dev
sudo python3 -m pip install wheel
sudo python3 -m pip install pillow
sudo python3 -m pip install pyserial
sudo python3 -m pip install numpy
```

https://github.com/joosteto/ws2812-spi
https://github.com/joosteto/ws2812-spi/issues/2
http://www.orangepi.org/orangepibbsen/forum.php?mod=viewthread&tid=3318
https://github.com/doceme/py-spidev
https://forum.up-community.org/discussion/2141/tutorial-gpio-i2c-spi-access-without-root-permissions
https://github.com/jgarff/rpi_ws281x (Userspace Raspberry Pi PWM library for WS281X LEDs)
https://oshlab.com/orange-pi-zero-pinout/

sudo python3 -m pip install spidev
sudo python3 -m pip install git+https://github.com/joosteto/ws2812-spi
(cd ~
git clone https://github.com/joosteto/ws2812-spi
cd ws2812-spi
sudo python3 setup.py install)

(python3 vs python2 -ongelma?)
/home/kurkku/ws2812-spi/ws2812.py
print str(err)
SyntaxError: invalid syntax
toimii: print (str(err)))

sudo sed -i 's/str(err)/(str(err))/g' /usr/local/lib/python3.5/dist-packages/ws2812.py

test spi:
import spidev
spi = spidev.SpiDev()
spi.open(1, 0)
to_send = [0x01, 0x02, 0x03]
spi.xfer(to_send)

test ws2812:
import spidev
import ws2812
spi = spidev.SpiDev()
spi.open(1,0)

#write 4 WS2812's, with the following colors: red, green, blue, yellow
ws2812.write2812(spi, [[10,0,0], [0,10,0], [0,0,10], [10, 10, 0]])

https://i.stack.imgur.com/O03j0.jpg
5V
MOSI (SPI1 MOSI/GPIO15/Pin19)
GND

https://docs.python.org/2/library/colorsys.html
https://stackoverflow.com/questions/24852345/hsv-to-rgb-color-conversion
import numpy as np
import colorsys

data = np.zeros((50,3), dtype=int)

# while 1:
for i in range(50):
  data[i] = colorsys.hsv_to_rgb(i/50,1,1)
ws2812.write2812(spi, data)
time.sleep(0.5)

data = np.array([[10,0,0], [0,10,0], [0,0,10], [10, 10, 0]])
ws2812.write2812(spi, data)

for i in range(50): data[i] = array(colorsys.hsv_to_rgb(i/50,1,1))*255
