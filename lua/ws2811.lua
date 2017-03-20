-- ws2811.lua

require("hsvToRgb")

strip = {}

PIXELCOUNT = 50

local buffer = ws2812.newBuffer(PIXELCOUNT, 3)
--local i = 0

function strip.rainbow(brightness, speed)
  if not brightness then brightness = 0.5 end
  if not speed then speed = 100 end
  for pixel = 0, PIXELCOUNT-1 do
    buffer:set(pixel+1, hsvToRgb(pixel/PIXELCOUNT, 1.0, brightness))
  end
  tmr.alarm(2, speed, 1, function()
    --i = i + 1
    buffer:shift(1, ws2812.SHIFT_CIRCULAR)
    ws2812.write(buffer)
  end)
end

function strip.color(r, g, b)
  buffer:fill(r, g, b)
  ws2812.write(buffer)
end

function strip.off()
  tmr.stop(2)
  buffer:fill(0, 0, 0)
  ws2812.write(buffer)
end

ws2812.init()
