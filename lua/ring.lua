-- ring.lua

local gnd = 5 -- D5
local btn = 6 -- D6
local brightness = 50
lock = false
gpio.mode(gnd, gpio.OUTPUT)
gpio.write(gnd, gpio.LOW)

gpio.mode(btn, gpio.INT, gpio.PULLUP)

gpio.trig(btn, "both", function()
    if not lock then
        lock = true
        tmr.create():alarm(50, tmr.ALARM_SINGLE, function()
            lock = false
            if gpio.read(btn) == gpio.LOW then
                ws2812_effects.set_delay(10)
                ws2812_effects.set_brightness(brightness)
            else
                ws2812_effects.set_brightness(0)
                ws2812_effects.set_delay(100)
            end
          end)
    end
  end)


-- init the ws2812 module
ws2812.init(ws2812.MODE_SINGLE)
-- create a buffer, 24 LEDs with 3 color bytes
strip_buffer = ws2812.newBuffer(24, 3)
-- init the effects module, set color to red and start blinking
ws2812_effects.init(strip_buffer)
-- speed 0 - 255, 255 = fastest
ws2812_effects.set_speed(255)
-- delay, 10ms = fastest, default 100ms? (rainbow 10ms?)
ws2812_effects.set_delay(10)
-- brightness 0 - 255, 0 = black
ws2812_effects.set_brightness(0)
ws2812_effects.set_mode("rainbow")
ws2812_effects.start()
