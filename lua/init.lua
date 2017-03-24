-- init.lua

local function abortStartup()
  tmr.stop(0)
  uart.on("data")
  print("startup aborted")
end

local function normalStartup()
  uart.on("data")
  print("Normal startup")
  dofile("main.lua")
end

local function init()
  wifi.setmode(wifi.STATION)
  w = wifi.sta.config -- do shortcut for wifi-settings
  print(); print()
  print("***")
  print("Press ENTER (\"\\r\") to abort startup")
  print("Change Wifi-settings: wifi.sta.config(\"ssid\",\"password\")")
  print("or: w(\"ssid\",\"password\")")
  print("***")
  uart.on("data", "\r", abortStartup, 0)
  tmr.alarm(0, 3000, 0, normalStartup)
end
    
tmr.alarm(0, 2000, 0, init)
