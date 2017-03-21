--mytimer = tmr.create():alarm(1000, 1, function() print(wifi.sta.getip()) end)



-- Wifi credentials
wifi.setmode(wifi.STATION)
--wifi.sta.config("ssid","password")

dofile("mqtt.lua");

dofile("ws2811.lua");

strip.off()

init_mqtt( function(conn, topic, input)
    print("mqtt callback")
    if string.upper(input) == "ON" then 
      strip.rainbow()
    elseif string.upper(input) == "OFF" then 
      strip.off()
    elseif string.upper(input) == "KITT" then
      strip.kitt(0.8, 20, 255, 0, 0)
    elseif string.upper(input) == "RAINBOW" then
      strip.rainbow()
    else
      --print("execute mqtt input string (type: " .. type(input) .. ")")
      --loadstring(input)
      print("mqtt input: " .. input)
      print("mqtt input type: " .. type(input))
      a = tonumber(input)
      --print(a)
      print("mqtt input tonumber: " .. a)
      print("mqtt input tonumber type: " .. type(a))
      --strip.set(a)
      strip.location(a)
    end 
  end)

