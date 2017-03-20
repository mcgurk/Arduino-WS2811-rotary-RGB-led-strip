### MQTT
```
mosquitto_pub -u xxxxx -P xxxx -h rasp -d -t ws2811 -m "ON"
mosquitto_pub -u xxxxx -P xxxx -h rasp -d -t ws2811 -m "OFF"
```

### NodeMcu firmware
- https://nodemcu-build.com/
- https://nodemcu.readthedocs.io/en/master/

### Flash (Windows):
https://github.com/marcelstoer/nodemcu-pyflasher/releases/download/v0.2.0/NodeMCU-PyFlasher-0.2.0.exe

COM15, nodemcu-master-24-modules-2017-03-15-09-23-08-float.bin, 921600, Dual Flash I/O (dio), wipes all data
(erase took 9,4s)

### Start with serial monitor
Putty, serial, COM15, 115200:
NodeMCU custom build by frightanic.com
        branch: master
        commit: b96e31477ca1e207aa1c0cdc334539b1f7d3a7f0
        SSL: false
        modules: adc,bit,dht,encoder,enduser_setup,file,gpio,http,i2c,mdns,mqtt,net,node,ow,pwm,rfswitch,rotary,spi,struct,tmr,uart,wifi,ws2801,ws2812
 build  built on: 2017-03-15 09:22
 powered by Lua 5.1.4 on SDK 2.0.0(656edbf)
lua: cannot open init.lua
>

### Wifi-settings (saved automatically)
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


### Send stuff to ESP without Lua-editor (needs Python)
https://github.com/4refr0nt/luatool

(with luatool script can update also over network)

### Send stuff from serial monitor
http://www.roboremo.com/luapaste.html

### Send 2-line file from serial monitor
```
s="hello.lua";file.remove(s);file.open(s,"w+");file.writeline([[print("hello nodemcu")]]);file.writeline([[print(node.heap())]]);file.close();
```


### Misc

In Lua, any value may represent a condition. Conditionals (such as the ones in control structures) consider false and nil as false and anything else as true. Beware that, unlike some other scripting languages, Lua considers both zero and the empty string as true in conditional tests.

Compiled file can be bigger than uncompiled script.

Still compiled might consume less heap.

Script: .lua, Compiled: .lc

http://files.catwell.info/misc/mirror/lua-5.2-bytecode-vm-dirk-laurie/lua52vm.html

