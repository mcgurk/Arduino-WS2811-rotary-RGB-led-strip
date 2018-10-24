// https://github.com/Makuna/NeoPixelBus
// https://github.com/Makuna/NeoPixelBus/wiki/NeoPixelBus-object-API
// https://github.com/Makuna/NeoPixelBus/wiki/HslColor-object-API
// http://www.vagrearg.org/content/hsvrgb

#include <NeoPixelBus.h>
//#define HSV_USE_ASSEMBLY  /* Optimize code using assembly */
// If assembly is used, Arduino IDE must be modified!:
// C:\Program Files (x86)\Arduino\hardware\arduino\avr\platform.txt
// compiler.cpp.flags: -OS -> -O1
#include "fast_hsv2rgb.c"

#define MAX_PIXELS 100
#define MAX_BRIGHTNESS 128
#define SPEED 6.0f
#define PERIODS 2.0f
// (100 ledstrip: 100, 255, 0.1, 1)
// (300 ledstrip: 300, 128, 2, 2)

const uint8_t PixelPin = 2;
uint16_t frame = 0;
float phase = 0;
uint32_t old_t = 0;

NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(MAX_PIXELS, PixelPin);

void setup() {
}

void loop() {
  //float hue_moving = ((float)(frame*SPEED))/360.0f * HSV_HUE_MAX;
  uint16_t hue_moving = phase * HSV_HUE_MAX;
  float hue_mul = HSV_HUE_MAX/(MAX_PIXELS/PERIODS);
  uint8_t *p = strip.Pixels();
  for (uint16_t i = 0; i < MAX_PIXELS; i++) {
    //float hue_temp = ((float)i)*hue_mul;
    //uint16_t hue = ((uint16_t)(hue_temp + hue_moving)) % HSV_HUE_MAX;
    uint16_t hue_temp = ((float)i)*hue_mul;
    uint16_t hue = (hue_temp + hue_moving) % HSV_HUE_MAX;
    fast_hsv2rgb_32bit(hue, 255, MAX_BRIGHTNESS, p++, p++, p++);
    //fast_hsv2rgb_8bit(hue, 255, MAX_BRIGHTNESS, p++, p++, p++);
    //*p=i; p++; *p=i; p++; *p=i; p++;
  }
  strip.Dirty();
  strip.Show();

  frame++;
  phase += SPEED/((float)(micros()-old_t));
  old_t = micros();
  phase = fmod(phase, 1.0f);
}
