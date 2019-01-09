## Schematics
- Wemos D1 (ESP8266, 4MB)
- 24 WS2812 RGB led ring
- 5V -> VCC
- D4 (GPIO2, blue led, must be high during bootup) -> Data
- GND -> GND
- D5 (GPIO14) -> Button ("gnd")
- D6 (GPIO12) -> Button (input, pullup)

https://nodemcu.readthedocs.io/en/master/en/modules/ws2812/

The library uses UART1 routed on GPIO2 (Pin D4 on NodeMCU DEVKIT) to generate the bitstream. It can use UART0 routed to TXD0 as well to handle two led strips at the same time.

### MQTT
```
mosquitto_pub -u xxxxx -P xxxx -h rasp -d -t ws2811 -m "ON"
mosquitto_pub -u xxxxx -P xxxx -h rasp -d -t ws2811 -m "OFF"
```

Retain so command is received after boot or desubscribe:
```
mosquitto_pub -u xxxxx -P xxxx -h rasp -d -t ws2811 -r -m "ON"
mosquitto_pub -u xxxxx -P xxxx -h rasp -d -t ws2811 -r -m "OFF"
```

MQTT commands:
- Off: off
- K.I.T.T: kitt
- Traveller: Traveller[,r,g,b]
- Rainbow: rainbow[,brightness(0-1.0)]
- Point: point[,r,g,b]
- Location of point (if point-mode): just number
- All same color without animation: color[,r,g,b]
- Status from status-topic: ping
- Save pixelcount: save,number_of_pixels (updates only after next mode-change or restart)


### NodeMcu firmware
- https://nodemcu-build.com/
- https://nodemcu.readthedocs.io/en/master/

### Flash (Windows):
https://github.com/marcelstoer/nodemcu-pyflasher/releases/download/v0.2.0/NodeMCU-PyFlasher-0.2.0.exe

COM15, nodemcu-master-24-modules-2017-03-15-09-23-08-float.bin, 921600, Dual Flash I/O (dio), wipes all data
(erase took 9,4s)

### Start with serial monitor
Putty, serial, COM15, 115200:
```
NodeMCU custom build by frightanic.com
        branch: master
        commit: b96e31477ca1e207aa1c0cdc334539b1f7d3a7f0
        SSL: false
        modules: adc,bit,dht,encoder,enduser_setup,file,gpio,http,i2c,mdns,mqtt,net,node,ow,pwm,rfswitch,rotary,spi,struct,tmr,uart,wifi,ws2801,ws2812
 build  built on: 2017-03-15 09:22
 powered by Lua 5.1.4 on SDK 2.0.0(656edbf)
lua: cannot open init.lua
>
```

### Wifi-settings (saved automatically to flash)
```
wifi.sta.config("ssid","password")
```

### Coding
http://esp8266.ru/esplorer-latest/?f=ESPlorer.zip

```
-- Blink:
ledPin = 4 -- gpio2
gpio.mode(ledPin, gpio.OUTPUT)
lighton=0
tmr.alarm(0,1000,1,function()
if lighton==0 then
    lighton=1
    gpio.write(ledPin, gpio.HIGH)
else
    lighton=0
    gpio.write(ledPin, gpio.LOW)
end
end)
print("Blinking")
-- tmr.stop(0)
```

Send to ESP

### Interrupt init.lua during boot
https://bigdanzblog.wordpress.com/2015/04/24/esp8266-nodemcu-interrupting-init-lua-during-boot/

### Send stuff to ESP without Lua-editor (needs Python)
https://github.com/4refr0nt/luatool

(with luatool script can update also over network)

### Send stuff from serial monitor
http://www.roboremo.com/luapaste.html

### Send 2-line file from serial monitor
```
s="hello.lua";file.remove(s);file.open(s,"w+");file.writeline([[print("hello nodemcu")]]);file.writeline([[print(node.heap())]]);file.close();
```

### ESP tool (Windows)
https://github.com/espressif/esptool

Install Python and then install esptool with:
```
pip install esptool
```

#### Read Wifi-settings
```
esptool.py --port COM15 read_flash 0x3fe000 128 wifisettings.bin
```
Address is end of flash minus 8192 (0x2000) bytes: 
- 512kB (0x80000) - 8192B (0x2000) = 516096B (0x7e000)
- 4096kB (0x400000) - 8192B (0x2000) = 4186112B (0x3fe000)

#### Erase Wifi-settings
Do blank file (128 bytes of 0xff):
```
dd if=/dev/zero bs=128 count=1 | tr '\000' '\377' > blank.bin
```
Fill with 0xff (4096kB flash):
```
C:\Users\[user]\AppData\Local\Arduino15\packages\esp8266\tools\esptool\0.4.9\esptool.exe -cp COM15 -cd nodemcu -cb 115200 -ca 0x3fe000 -cf blank.bin
```
or with esptool.py:
```
esptool.py --port COM15 write_flash 0x3fe000 blank.bin
```
### Tables
Split string to table:
```
input = "fds,sdsgsdf,sdfasd,asdas"
local t={}
for x in string.gmatch(input, "([^,]+)") do table.insert(t, x) end
```
Print table:
```
for key,value in pairs(t) do print(key,value) end
```

### Misc

In Lua, any value may represent a condition. Conditionals (such as the ones in control structures) consider false and nil as false and anything else as true. Beware that, unlike some other scripting languages, Lua considers both zero and the empty string as true in conditional tests.

Compiled file can be bigger than uncompiled script.

Still compiled might consume less heap.

Script: .lua, Compiled: .lc

http://files.catwell.info/misc/mirror/lua-5.2-bytecode-vm-dirk-laurie/lua52vm.html

### Commands
- https://iotbytes.wordpress.com/managing-files-with-nodemcu/
- https://nodemcu.readthedocs.io/en/master/en/modules/file/
```
-- cat("filename"):
function cat(filename) file.open(filename) while true do line = file.readline() if (line == nil) then file.close() break end line = string.gsub(line, "[\r\n]", "") print(line) end end

-- ls():
ls = function() for k,v in pairs(file.list()) do print("name:"..k..", size:"..v) end end

-- df():
df = function() local r, u, t = file.fsinfo() print("\nFile system info:\nTotal : "..t.."\nUsed : "..u.."\nRemain: "..r.."\n") end

-- dir():
dir = function() ls() df() end
```
