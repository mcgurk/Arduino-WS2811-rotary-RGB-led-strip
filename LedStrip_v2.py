#!/usr/bin/python3

# sudo python3 -m pip install paho-mqtt
# sudo python3 -m pip install parse

# myconfig.py:
# mqtt_username = "username"
# mqtt_password = "password"
# mqtt_broker = "broker_address"
# mqtt_topic = "something/else"

import time
import paho.mqtt.client as paho
import numpy as np
import serial
from parse import parse
from myconfig import *

def fill(r, g, b):
  #data = np.full((300, 3), [r, g, b], dtype=np.uint8)
  data = np.full((300, 3), [g, r, b], dtype=np.uint8)
  ser.write(data)

def rainbow():
  ser.write(b'{"mode":"rainbow"}')

def on_message(client, userdata, message):
  msg = str(message.payload.decode("utf-8"))
  print("received message =", msg)
  print(msg.upper())
  if msg.upper() == "OFF":
    print("off!")
    #fill(0, 0, 0)
    ser.write(b'{"mode":"off"}')
  if msg.upper() == "RAINBOW":
    print("rainbow!")
    #rainbow()
    ser.write(b'{"mode":"rainbow"}')
  koe = parse("fill {:d} {:d} {:d}", msg)
  print(koe)
  if koe:
    print("fill")
    fill(koe[0], koe[1], koe[2])

ser = serial.Serial('/dev/ttyACM0', 2000000)

client = paho.Client("client-001")
client.on_message = on_message
client.username_pw_set(username=mqtt_username, password=mqtt_password)
print("connecting to broker ", mqtt_broker)
client.connect(mqtt_broker) #connect
client.loop_start() #start loop to process received messages
print("subscribing ")
client.subscribe(mqtt_topic) #subscribe

# SIGINT will normally raise a KeyboardInterrupt, just like any other Python call
try:
  while 1:
    #print("joo")
    time.sleep(1)
except KeyboardInterrupt:
  print("W: interrupt received, stopping…")
finally:
  # clean up
  print("cleanup")
  ser.close()
  client.disconnect() #disconnect
  client.loop_stop() #stop loop

