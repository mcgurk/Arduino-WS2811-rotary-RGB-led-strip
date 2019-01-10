import machine, time, neopixel, sys, json
print("--- WS2812 LED String lightpainting system by McGurk ---")

default_config = {'speed':4000, 'brightness':0.25, 'sleep':0.01, 'pixels':24, 'pin':4, 'gamma':1}


def loadconfig(default = {}):
  newconfig = default.copy()
  print("default config:", default)
  try:
    file = open("config.json", "r")
    loaded = json.load(file)
    print("loaded config:", loaded)
    newconfig.update(loaded)
  except(OSError, ValueError):
    print("Couldn't open/parse config file")
  print("active config:", newconfig)
  return newconfig

config = loadconfig(default_config)

gamma_table = (0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,3,3,3,3,3,3,3,4,4,4,4,4,5,5,5,5,6,6,6,6,7,7,7,7,8,8,8,9,9,9,10,10,10,11,11,11,12,12,13,13,13,14,14,15,15,16,16,17,17,18,18,19,19,20,20,21,21,22,22,23,24,24,25,25,26,27,27,28,29,29,30,31,32,32,33,34,35,35,36,37,38,39,39,40,41,42,43,44,45,46,47,48,49,50,50,51,52,54,55,56,57,58,59,60,61,62,63,64,66,67,68,69,70,72,73,74,75,77,78,79,81,82,83,85,86,87,89,90,92,93,95,96,98,99,101,102,104,105,107,109,110,112,114,115,117,119,120,122,124,126,127,129,131,133,135,137,138,140,142,144,146,148,150,152,154,156,158,160,162,164,167,169,171,173,175,177,180,182,184,186,189,191,193,196,198,200,203,205,208,210,213,215,218,220,223,225,228,231,233,236,239,241,244,247,249,252,255)

def gamma(c):
  return (gamma_table[c[0]], gamma_table[c[1]], gamma_table[c[2]])

def clear():
  for x in range(np.n): np[x] = (0,0,0)
  np.write()

def test(b = config['brightness']):
  for x in range(np.n):
    np[x] = hsv_to_rgb(x/np.n, 1, b)
  np.write()

def hsv_to_rgb_normal(h, s, v):
  if s == 0.0: v*=255; return (v, v, v)
  i = int(h*6.)
  f = (h*6.)-i; p,q,t = int(255*(v*(1.-s))), int(255*(v*(1.-s*f))), int(255*(v*(1.-s*(1.-f)))); v*=255; i%=6
  v = int(v)
  if i == 0: return (v, t, p)
  if i == 1: return (q, v, p)
  if i == 2: return (p, v, t)
  if i == 3: return (p, q, v)
  if i == 4: return (t, p, v)
  if i == 5: return (v, p, q)

def hsv_to_rgb_gamma(h, s, v):
  if s == 0.0: v*=255; return (v, v, v)
  i = int(h*6.)
  f = (h*6.)-i; p,q,t = int(255*(v*(1.-s))), int(255*(v*(1.-s*f))), int(255*(v*(1.-s*(1.-f)))); v*=255; i%=6
  v = gamma_table[int(v)]
  t = gamma_table[t]
  p = gamma_table[p]
  q = gamma_table[q]
  if i == 0: return (v, t, p)
  if i == 1: return (q, v, p)
  if i == 2: return (p, v, t)
  if i == 3: return (p, q, v)
  if i == 4: return (t, p, v)
  if i == 5: return (v, p, q)

if config['gamma'] == 1:
  hsv_to_rgb = hsv_to_rgb_gamma
else:
  hsv_to_rgb = hsv_to_rgb_normal

def setup():
  newconfig = loadconfig(default_config)
  print(" \'-\' = load default value")
  for key in default_config:
    while True:
      value = input("Give "+key+" (default:"+str(default_config[key])+") ["+str(newconfig[key])+"]: ")
      if value != '' and value != '-':
        try:
          newconfig[key] = float(value)
        except(ValueError):
          print("Must be number. Use point (.) as decimal separator.")
          continue
        break
      if value == '': break
      if value == '-':
        del(newconfig[key])
        break
  try:
    print("trying to save config:", newconfig)
    file = open("config.json", "w")
    json.dump(newconfig, file)
    file.close()
    print("config saved")
  except:
    print("config saving failed")

np = neopixel.NeoPixel(machine.Pin(config['pin']), config['pixels']) # D2
machine.Pin(12, machine.Pin.OUT).value(0) # D6 = LOW/GND
pin = machine.Pin(14, machine.Pin.IN, machine.Pin.PULL_UP) # D5
while True:
  t = (time.ticks_ms() / config['speed']) % 1.0
  c = (0,0,0) if pin.value() else hsv_to_rgb(t, 1, config['brightness'])
  for x in range(np.n): np[x] = c
  np.write()
  time.sleep(config['sleep'])
