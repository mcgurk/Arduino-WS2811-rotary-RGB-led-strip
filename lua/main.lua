--mytimer = tmr.create():alarm(1000, 1, function() print(wifi.sta.getip()) end)

-- Wifi credentials
wifi.setmode(wifi.STATION)
--wifi.sta.config("ssid","password")

dofile("mqtt.lua");

dofile("ws2811.lua");

strip.off()

init_mqtt( function(conn, topic, input) 
    if input == "ON" then 
      strip.rainbow()
    else 
      strip.off()
    end 
  end)

