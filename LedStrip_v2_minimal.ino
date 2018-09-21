#include <NeoPixelBus.h>

NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(150, 2);
uint8_t *ptr;

void setup() {
  Serial.begin(2000000);
  strip.Begin();
  ptr = strip.Pixels();
}

void loop() {
  if (Serial.available()) {
    Serial.readBytes(ptr, 150*3);
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
imgarr = np.array(Image.open("/misc/valo.png").convert("RGB"))

i = 0
while 1:
  line = i % 300
  ser.write(imgarr[line].flatten())
  time.sleep(0.02)
  i += 1
*/
