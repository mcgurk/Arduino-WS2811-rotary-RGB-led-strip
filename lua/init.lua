-- init.lua

function abortInit()
  abort = false
  print("Press ENTER to abort startup")
  print("Change Wifi-settings: wifi.sta.config(\"ssid\",\"password\")")
  -- if <CR> is pressed, call abortTest
  uart.on("data", "\r", abortTest, 0)
  tmr.alarm(0,3000,0,startup)
end
    
function abortTest(data)
  abort = true
  uart.on("data")
end

function startup()
  uart.on("data")
  if abort == true then
    print("startup aborted")
    return
  end
  print("Normal startup")
  dofile("main.lua")
end

tmr.alarm(0, 2000, 0, abortInit)
