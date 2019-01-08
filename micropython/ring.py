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

import machine, time, neopixel
np = neopixel.NeoPixel(machine.Pin(4), 24) # D2
machine.Pin(12, machine.Pin.OUT).value(0) # D6 = LOW/GND
pin = machine.Pin(14, machine.Pin.IN, machine.Pin.PULL_UP) # D5
while True:
  t = (time.ticks_ms() / 5000) % 1.0
  c = (0,0,0) if pin.value() else hsv_to_rgb(t, 1, 0.25)
  for x in range(24): np[x] = c
  np.write()
  time.sleep(0.02)
