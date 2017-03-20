dofile("mqtt.lua");

dofile("ws2811.lua");

init_wifi_mqtt( function(conn, topic, input) 
    if input == "ON" then 
      strip.rainbow()
    else 
      strip.off()
    end 
  end)
