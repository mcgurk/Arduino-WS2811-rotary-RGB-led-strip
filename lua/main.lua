dofile("mqtt.lua");

dofile("ws2811.lua");

init_wifi_mqtt( function(conn, topic, input) 
    if input == "ON" then 
      update_ws2811(0.5) 
    else 
      update_ws2811(0.0) 
    end 
  end)
  
  
