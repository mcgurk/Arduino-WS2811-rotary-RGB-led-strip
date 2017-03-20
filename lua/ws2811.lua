-- ws2811.lua

PIXELS = 50

local function hsvToRgb(h, s, v)
  local r, g, b

  local i = math.floor(h * 6);
  local f = h * 6 - i;
  local p = v * (1 - s);
  local q = v * (1 - f * s);
  local t = v * (1 - (1 - f) * s);

  i = i % 6

  if i == 0 then r, g, b = v, t, p
  elseif i == 1 then r, g, b = q, v, p
  elseif i == 2 then r, g, b = p, v, t
  elseif i == 3 then r, g, b = p, q, v
  elseif i == 4 then r, g, b = t, p, v
  elseif i == 5 then r, g, b = v, p, q
  end

  return r * 255, g * 255, b * 255
end

--function init_ws2811()
ws2812.init()
--buffer = ""
--buffer = ws2812.newBuffer(50, 3);
--buffer:fill(0, 0, 0);
--ws2812.write(buffer)

--end

local i = 0
local buffer = ws2812.newBuffer(50, 3)
--buffer:fill(0, 0, 0)
--buffer:set(1, 255, 255, 255)
function update_ws2811(brightness)
print("ws2811 update")
for pixel = 0, PIXELS-1 do
  buffer:set(pixel+1, hsvToRgb(pixel/PIXELS, 1.0, brightness))
  --local Hue = pixel/PIXELS
  --local temporary_R = Hue + 0.333
  --local temporary_G = Hue
  --local temporary_B = Hue - 0.333
  --buffer:set(pixel+1, temporary_R * 255.0, temporary_G*255.0, temporary_B*255.0)
  --buffer = buffer .. colourWheel((index + pixel * rainbow_speed) % 256)
end
--buffer:set(1, 255, 0, 0)
tmr.alarm(2, 100, 1, function()
  i = i + 1
  --buffer:fade(2)
  --buffer:set(i % buffer:size() + 1, 255, 255, 255)
  buffer:shift(1, ws2812.SHIFT_CIRCULAR)
  ws2812.write(buffer)
end)
end
