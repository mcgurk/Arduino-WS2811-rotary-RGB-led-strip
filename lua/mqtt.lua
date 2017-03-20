-- mqtt.lua

local broker = "xxxx.xxxx.xx"
local mqttport = 1883
local userID = "xxxx"
local userPWD  = "xxxx"
local clientID = "ESP1"
local topic = "ws2811"
local GPIO2 = 4
local ready = 0

gpio.mode(GPIO2, gpio.OUTPUT)

-- Wifi credentials
wifi.setmode(wifi.STATION)
--wifi.sta.config("ssid","password")

local function wifi_connect()
  ip = wifi.sta.getip()
  if ip ~= nil then
    print('Connected, ip is:' .. ip)
    tmr.stop(1)
    ready = 1
  else
    ready = 0
  end
end

local function mqtt_do(callback)
if ready == 1 then
  m = mqtt.Client(clientID, 120, userID, userPWD)
  m:connect(broker , mqttport, 0, 1,
    function(conn)
      print("Connected to MQTT:" .. broker .. ":" .. mqttport .." as " .. clientID )
      tmr.stop(0)
      connected = 1;
      m:subscribe(topic, 0, 
        function(conn)
          print('Subscribed')
        end)
      m:on('message', 
        function(conn, topic, input)
          print(input)
          if input == "ON" then
            gpio.write(GPIO2, gpio.HIGH)
          elseif input == "OFF" then
            gpio.write(GPIO2, gpio.LOW)
          else
            print('error')
          end
          callback(conn, topic, input)
       end)
  end)
end
end
    

function init_wifi_mqtt(callback)
  tmr.alarm(0, 1000, 1, 
    function() 
      mqtt_do(callback)
      tmr.delay(1000)
    end)
     
  tmr.alarm(1, 1111, 1, 
    function()
      wifi_connect() 
    end)
end

