// https://github.com/Makuna/NeoPixelBus
// https://github.com/Makuna/NeoPixelBus/wiki/NeoPixelBus-object-API
// https://github.com/Makuna/NeoPixelBus/wiki/HslColor-object-API
// http://www.vagrearg.org/content/hsvrgb

#include <NeoPixelBus.h>
#include "fast_hsv2rgb.c"

#define MAX_PIXELS 300
#define MAX_BRIGHTNESS 128
#define SPEED 1
#define PERIODS 2
// (100 ledstrip: 100, 255, 0.1, 1)
// (300 ledstrip: 300, 128, 2, 2)

const uint8_t PixelPin = 2;
uint16_t frame = 0;

NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(MAX_PIXELS, PixelPin);

void setup() {
}

void loop() {
  //float hue_moving = ((float)(frame*SPEED))/360.0f * HSV_HUE_MAX;
  float hue_moving = ((float)(frame*SPEED))/MAX_PIXELS * HSV_HUE_MAX;
  float hue_mul = HSV_HUE_MAX/(MAX_PIXELS/PERIODS);
  uint8_t *p = strip.Pixels();
  for (uint16_t i = 0; i < MAX_PIXELS; i++) {
    float hue_temp = ((float)i)*hue_mul;
    uint16_t hue = ((uint16_t)(hue_temp + hue_moving)) % HSV_HUE_MAX;
    fast_hsv2rgb_32bit(hue, 255, MAX_BRIGHTNESS, p++, p++, p++);
  }
  strip.Dirty();
  strip.Show();
  frame++;
  //if (frame == 360) frame = 0;
  if (frame == (MAX_PIXELS/SPEED)) frame = 0;
}

