#!/usr/bin/python3

# myconfig.py:
# mqtt_username = "username"
# mqtt_password = "password"
# mqtt_broker = "broker_address"

import time
import paho.mqtt.client as paho
import numpy as np
import serial
from myconfig import *

def fill(r, g, b):
  data = np.full((600, 3), [r, g, b], dtype=np.uint8)
  ser.write(data)

def rainbow():
  ser.write(b'r')

def on_message(client, userdata, message):
  print("received message =",str(message.payload.decode("utf-8")))
  print(message.payload.upper())
  if message.payload.upper() == b"CLEAR":
    print("clear!")
    fill(0, 0, 0)
  if message.payload.upper() == b"RAINBOW":
    print("rainbow!")
    rainbow()

ser = serial.Serial('/dev/ttyACM0', 2000000)

client = paho.Client("client-001")
client.on_message = on_message
client.username_pw_set(username=mqtt_username, password=mqtt_password)
print("connecting to broker ", mqtt_broker)
client.connect(mqtt_broker) #connect
client.loop_start() #start loop to process received messages
print("subscribing ")
client.subscribe("house/bulb1") #subscribe

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
