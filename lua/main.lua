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
    cmd = string.upper(input)
    if cmd == "ON" then 
      strip.rainbow()
    elseif cmd == "OFF" then 
      strip.off()
    elseif cmd == "KITT" then
      strip.kitt(0.8, 20, 255, 0, 0)
    elseif cmd == "RAINBOW" then
      strip.rainbow()
    elseif cmd == "POINT" then
      strip.point()
    elseif cmd == "PING" then
      reply_mqtt_ping()
    else
      print("Unknown command")
      --[[--print("execute mqtt input string (type: " .. type(input) .. ")")
      --loadstring(input)
      a = tonumber(input)
      --print(a)
      print("mqtt input tonumber: " .. a)
      print("mqtt input tonumber type: " .. type(a))
      --strip.set(a)
      strip.location(a)]]
    end 
  end)

