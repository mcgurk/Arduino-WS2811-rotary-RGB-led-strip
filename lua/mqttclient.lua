-- mqttclient.lua

local mqttclient = {}

local broker = "xxxx.xxxx.xx"
local mqttport = 1883
local userID = "xxxxx"
local userPWD  = "xxxxx"
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
            print("Subscribed to *" .. topic .. "*")
          end)
        m:on('message', 
          function(conn, topic, input)
            print("mqtt message from topic *" .. topic .. "*: *" .. input .. "*")
            if string.upper(input) == "PING" then
              m:publish("status", clientID .. " ping reply! (flash size: " .. node.flashsize() .. ", heap: " .. node.heap() .. ")", 0, 0)
            else
              callback(conn, topic, input)
            end
          end)
      end)
  end
end
    
function mqttclient.send_status(status)
  print(status)
  m:publish("status", status, 0, 0)
end

function mqttclient.start(callback)
  print("init_mqtt")
  if not mqtt_connected then
    tmr.alarm(0, 1000, 1, 
      function() 
        mqtt_do(callback)
      end)
  end
end

return mqttclient
