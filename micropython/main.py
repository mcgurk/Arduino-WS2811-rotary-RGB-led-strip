import machine, time, neopixel, sys
np = neopixel.NeoPixel(machine.Pin(4), 24) # D2
machine.Pin(12, machine.Pin.OUT).value(0) # D6 = LOW/GND
pin = machine.Pin(14, machine.Pin.IN, machine.Pin.PULL_UP) # D5
if pin.value() == 0: sys.exit()
while True:
  t = (time.ticks_ms() / 5000) % 1.0
  c = (0,0,0) if pin.value() else hsv_to_rgb(t, 1, 0.25)
  for x in range(24): np[x] = c
  np.write()
  time.sleep(0.01)
