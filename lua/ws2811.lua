-- ws2811.lua

require("hsvToRgb")

strip = {}

PIXELCOUNT = 50

local buffer = ws2812.newBuffer(PIXELCOUNT, 3)
local i = 1
local d = true

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

function strip.set(location, r, g, b)
  if not location or location < 1 then location = 1 end
  if not r then r = 128 g = 128 b = 128 end
  if location > PIXELCOUNT then location = PIXELCOUNT end
  buffer:set(location, r, g, b)
end

function strip.fade(factor, speed)
  if not factor then factor = 0.5 end
  if not speed then speed = 100 end
  tmr.alarm(2, speed, 1, function()
    buffer:mix(factor * 255, buffer)
    ws2812.write(buffer)
  end)
end

function strip.kitt(fadefactor, speed, r, g, b)
  if not fadefactor then fadefactor = 0.5 end
  if not speed then speed = 100 end
  if not r then r = 255 g = 0 b = 0 end
  d = true
  i = 1
  tmr.alarm(2, speed, 1, function()
    buffer:mix(fadefactor * 255, buffer)
    buffer:set(i, r, g, b)
    if d then i = i + 1 else i = i - 1 end
    if i < 1 then d = true i = 1 end
    if i > PIXELCOUNT then d = false i = PIXELCOUNT end
    ws2812.write(buffer)
  end)
end

ws2812.init()
