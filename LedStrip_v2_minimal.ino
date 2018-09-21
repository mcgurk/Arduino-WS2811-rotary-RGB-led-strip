#include <NeoPixelBus.h>

NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(150, 2);

void setup() {
  Serial.begin(2000000);
  strip.Begin();
}

void loop() {
  if (Serial.available()) {
    char *ptr = strip.Pixels();
    Serial.readBytes(ptr, 150*3);
    strip.Dirty();
    strip.Show();
  }
  delay(4);
}
