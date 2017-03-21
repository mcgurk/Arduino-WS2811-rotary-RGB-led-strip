-- main.lua

-- Wifi credentials
wifi.setmode(wifi.STATION)
--wifi.sta.config("ssid","password")

dofile("mqtt.lua");

dofile("ws2811.lua");

strip.off()

init_mqtt( function(conn, topic, input)
    print("mqtt callback")
    print("mqtt input: " .. input)
    print("mqtt input type: " .. type(input))
    local a = tonumber(input)
    if a then 
      print("number")
      strip.location(a)
      return
    end
    print("not number")
    local t={}
    for x in string.gmatch(input, "([^,]+)") do table.insert(t, x) end
    for key,value in pairs(t) do print(key,value) end
    cmd = string.upper(t[1])
    --table.remove(t,1)
    local n={}
    for key,value in pairs(t) do n[key] = tonumber(value) end
    print("command: " .. cmd)
    if cmd == "ON" then 
      strip.rainbow()
    elseif cmd == "OFF" then 
      strip.off()
    elseif cmd == "KITT" then
      strip.kitt(0.8, 20, 255, 0, 0)
    elseif cmd == "TRAVELLER" then
      strip.kitt(0.8, 20, n[2], n[3], n[4])
    elseif cmd == "RAINBOW" then
      strip.rainbow(t[2])
    elseif cmd == "POINT" then
      strip.point()
    elseif cmd == "COLOR" then
      strip.color(n[2], n[3], n[4])
    elseif cmd == "PING" then
      reply_mqtt_ping()
    else
      print("Unknown command")
      --[[--print("execute mqtt input string (type: " .. type(input) .. ")")
      --loadstring(input)]]
    end 
  end)

