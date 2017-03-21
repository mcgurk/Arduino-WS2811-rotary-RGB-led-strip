-- ws2811.lua

require("hsvToRgb")

strip = {}

PIXELCOUNT = 50

local buffer = ws2812.newBuffer(PIXELCOUNT, 3)
local i = 1
local d = true
local frame = 1
local location = 1

local function striptimer(callback, speed)
  if not speed then speed = 100 end
  tmr.alarm(2, speed, 1, function()
    callback()
    frame = frame + 1
    ws2812.write(buffer)
  end)
end

function strip.rainbow(brightness, speed)
  if not brightness then brightness = 0.5 end
  for pixel = 0, PIXELCOUNT-1 do
    buffer:set(pixel+1, hsvToRgb(pixel/PIXELCOUNT, 1.0, brightness))
  end
  striptimer(function()
    buffer:shift(1, ws2812.SHIFT_CIRCULAR)
  end, speed)
end

function strip.color(r, g, b)
  tmr.stop(2)
  if not r then r = 128 g = 128 b = 128 end
  buffer:fill(r, g, b)
  ws2812.write(buffer)
end

function strip.off()
  tmr.stop(2)
  buffer:fill(0, 0, 0)
  ws2812.write(buffer)
end

function strip.set(index, r, g, b)
  if not index or index < 1 then index = 1 end
  if index > PIXELCOUNT then index = PIXELCOUNT end
  if not r then r = 128 g = 128 b = 128 end
  buffer:set(index, r, g, b)
end

function strip.fade(factor, speed)
  if not factor then factor = 0.5 end
  if not speed then speed = 100 end
  striptimer(function()
    buffer:mix(factor * 255, buffer)
  end, speed)
end

function strip.point(factor, speed, r, g, b)
  if not factor then factor = 0.5 end
  if not speed then speed = 100 end
  striptimer(function()
    buffer:mix(factor * 255, buffer)
    strip.set(location, r, g, b)
  end, speed)
end

function strip.location(newlocation)
  location = newlocation
end

function strip.kitt(factor, speed, r, g, b)
  if not factor then factor = 0.5 end
  --if not r then r = 255 g = 0 b = 0 end
  d = true
  i = 1
  striptimer(function()
    buffer:mix(factor * 255, buffer)
    --buffer:set(i, r, g, b)
    --buffer:set(i, hsvToRgb((frame%1024)/1024.0, 1.0, 1.0))
    if not r then 
      strip.set(i, hsvToRgb((frame%1024)/1024.0, 1.0, 1.0))
    else
      strip.set(i, r, g, b)
    end
    if d then i = i + 1 else i = i - 1 end
    if i < 1 then d = true i = 1 end
    if i > PIXELCOUNT then d = false i = PIXELCOUNT end
  end, speed)
end


ws2812.init()
