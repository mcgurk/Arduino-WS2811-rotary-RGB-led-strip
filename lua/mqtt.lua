-- mqtt.lua

local broker = "xxxx.xxxx.xx"
local mqttport = 1883
local userID = "xxxx"
local userPWD  = "xxxx"
local clientID = "ESP-" .. string.format('%x', node.chipid())
local topic = "ws2811"
local mqtt_connected = false


local function mqtt_do(callback)
  print("mqtt_do")
  if wifi.sta.getip() then
    print('Connected, ip is:' .. wifi.sta.getip())
    if not m then 
      m = mqtt.Client(clientID, 120, userID, userPWD)
    else
      m:close()
    end
    m:connect(broker , mqttport, 0, 1,
      function(conn)
        print("Connected to MQTT:" .. broker .. ":" .. mqttport .." as " .. clientID )
        m:publish("status", clientID .. " connected! (flash size: " .. node.flashsize() .. ", heap: " .. node.heap() .. ")", 0, 0)
        tmr.stop(0)
        m:subscribe(topic, 0, 
          function(conn)
            print('Subscribed')
          end)
        m:on('message', 
          function(conn, topic, input)
            print(input)
            callback(conn, topic, input)
          end)
      end)
  end
end
    

function init_mqtt(callback)
  print("init_mqtt")
  if not mqtt_connected then
    tmr.alarm(0, 1000, 1, 
      function() 
        mqtt_do(callback)
        --tmr.delay(1000)
      end)
  end
end

