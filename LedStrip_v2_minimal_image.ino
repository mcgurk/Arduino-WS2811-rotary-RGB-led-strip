#include <NeoPixelBus.h>
#define PIXELS 150

NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(PIXELS, 2);
uint8_t *ptr;

void setup() {
  Serial.begin(2000000);
  strip.Begin();
  ptr = strip.Pixels();
}

void loop() {
  if (Serial.available()) {
    Serial.readBytes(ptr, PIXELS*3);
    strip.Dirty();
    strip.Show();
  }
  delay(4);
}

/* Python:
from PIL import Image
import numpy as np
import serial
import time

ser = serial.Serial('/dev/ttyACM0', 2000000)
imgarr = np.array(Image.open("/misc/valo.png").convert("RGB")) # image must be PIXELS wide
imgarr[:,:,[1,0]] = imgarr[:,:,[0,1]] # RGB -> GRB

i = 0
while 1:
  line = i % 300
  ser.write(imgarr[line].flatten())
  time.sleep(0.02)
  i += 1
*/
