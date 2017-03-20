-- ws2811.lua

require("hsvtorgb")

PIXELS = 50

local i = 0
local buffer = ws2812.newBuffer(50, 3)
--buffer:fill(0, 0, 0)
--buffer:set(1, 255, 255, 255)
function update_ws2811(brightness)
print("ws2811 update")
for pixel = 0, PIXELS-1 do
  buffer:set(pixel+1, hsvToRgb(pixel/PIXELS, 1.0, brightness))
end
--buffer:set(1, 255, 0, 0)
tmr.alarm(2, 100, 1, function()
  i = i + 1
  buffer:shift(1, ws2812.SHIFT_CIRCULAR)
  ws2812.write(buffer)
end)
end

ws2812.init()
