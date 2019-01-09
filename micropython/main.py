import machine, time, neopixel, sys, json
print("WS2812 LED String lightpainting system by McGurk")

default_config = {'speed':4000, 'brightness':0.25, 'sleep':0.01}

def hsv_to_rgb(h, s, v):
  if s == 0.0: v*=255; return (v, v, v)
  i = int(h*6.) # XXX assume int() truncates!
  f = (h*6.)-i; p,q,t = int(255*(v*(1.-s))), int(255*(v*(1.-s*f))), int(255*(v*(1.-s*(1.-f)))); v*=255; i%=6
  v = int(v)
  if i == 0: return (v, t, p)
  if i == 1: return (q, v, p)
  if i == 2: return (p, v, t)
  if i == 3: return (p, q, v)
  if i == 4: return (t, p, v)
  if i == 5: return (v, p, q)

def loadconfig(default = {}):
  print("default config:", default)
  try:
    file = open("config.json", "r")
    loaded = json.load(file)
    print("loaded config:", loaded)
    default.update(loaded)
  except(OSError, ValueError):
    print("Couldn't open/parse config file")
  print("new config:", default)
  return default

def setup():
  newconfig = loadconfig()
  print("0 = delete value from config.json")
  brightness = input("Give brightness: ")
  speed = input("Give speed: ")
  if brightness == '0': del(newconfig['brightness'])
  if speed == '0': del(newconfig['speed'])
  if brightness != '0' and brightness != '':
    try:
      newconfig['brightness'] = float(brightness)
    except(ValueError):
      print("Must be numbers. Use point (.) as decimal separator")
  if speed != '0' and speed != '':
    try:
      newconfig['speed'] = float(speed)
    except(ValueError):
      print("Must be numbers. Use point (.) as decimal separator")
  try:
    print("trying to save config:", newconfig)
    file = open("config.json", "w")
    json.dump(newconfig, file)
    file.close()
    print("config saved")
  except:
    print("config saving failed")

config = loadconfig(default_config)
np = neopixel.NeoPixel(machine.Pin(4), 24) # D2
machine.Pin(12, machine.Pin.OUT).value(0) # D6 = LOW/GND
pin = machine.Pin(14, machine.Pin.IN, machine.Pin.PULL_UP) # D5
while True:
  t = (time.ticks_ms() / config['speed']) % 1.0
  c = (0,0,0) if pin.value() else hsv_to_rgb(t, 1, config['brightness'])
  for x in range(24): np[x] = c
  np.write()
  time.sleep(config['sleep'])