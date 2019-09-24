// https://github.com/Makuna/NeoPixelBus
// https://github.com/Makuna/NeoPixelBus/wiki/NeoPixelBus-object-API
// https://github.com/Makuna/NeoPixelBus/wiki/HslColor-object-API

#include <NeoPixelBus.h>

#define PIXELS 150
#define BRIGHTNESS 150
#define PIXELS_H 100
#define BRIGHTNESS_H 255
#define VELOCITY 6.0f
#define PERIODS 1.0f

#define GAMMA

#ifdef GAMMA
const uint8_t gamma_table[] PROGMEM = \
{ 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, \
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1, \
  1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2, \
  2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  5,  5,  5, \
  5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  9,  9,  9, 10, \
 10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16, \
 17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25, \
 25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36, \
 37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50, \
 51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68, \
 69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89, \
 90, 92, 93, 95, 96, 98, 99,101,102,104,105,107,109,110,112,114, \
115,117,119,120,122,124,126,127,129,131,133,135,137,138,140,142, \
144,146,148,150,152,154,156,158,160,162,164,167,169,171,173,175, \
177,180,182,184,186,189,191,193,196,198,200,203,205,208,210,213, \
215,218,220,223,225,228,231,233,236,239,241,244,247,249,252,255 };
#endif

const uint8_t PixelPin = 2;
uint16_t frame = 0;
float phase = 0;
uint32_t old_t = 0;
uint16_t pixels = 0;
uint8_t brightness = 0;

NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(PIXELS, PixelPin);

void setup() {
  pinMode(8, INPUT_PULLUP);
  pinMode(16, INPUT_PULLUP);
  pinMode(7, OUTPUT);
  digitalWrite(7, LOW);
  pinMode(14, OUTPUT);
  digitalWrite(15, LOW);
  if (digitalRead(8) && digitalRead(16)) {
    pixels = PIXELS;
    brightness = BRIGHTNESS;
  } else {
    pixels = PIXELS_H;
    brightness = BRIGHTNESS_H;
  }
}

#define HSV_HUE_SEXTANT    256
#define HSV_HUE_STEPS   (6 * HSV_HUE_SEXTANT)
#define HSV_HUE_MIN   0
#define HSV_HUE_MAX   (HSV_HUE_STEPS - 1)
#define HSV_SAT_MIN   0
#define HSV_SAT_MAX   255
#define HSV_VAL_MIN   0
#define HSV_VAL_MAX   255

void loop() {
  uint16_t hue_moving = phase * HSV_HUE_MAX;
  float hue_mul = HSV_HUE_MAX/(pixels/PERIODS);
  uint8_t *ptr = strip.Pixels();
  for (uint16_t i = 0; i < pixels; i++) {
    uint16_t hue_temp = ((float)i)*hue_mul;
    uint16_t hue = (hue_temp + hue_moving) % HSV_HUE_MAX;
    fast_hsv2rgb_32bit(hue, 255, brightness, ptr++, ptr++, ptr++);
  }

  #ifdef GAMMA
  ptr = strip.Pixels(); 
  for (uint16_t i = 0; i < PIXELS*3; i++) {
    *ptr++ = pgm_read_byte_near(gamma_table + *ptr);
  }
  #endif

  strip.Dirty();
  strip.Show();

  frame++;
  phase += VELOCITY/((float)(micros()-old_t));
  old_t = micros();
  phase = fmod(phase, 1.0f);
}


/*
 * fast_hsv2rgb_32bit(uint16_t h, uint8_t s, uint8_t v, uint8_t *r, uint8_t *g , uint8_t *b)
 */
#define HSV_MONOCHROMATIC_TEST(s,v,r,g,b) \
  do { \
    if(!(s)) { \
       *(r) = *(g) = *(b) = (v); \
      return; \
    } \
  } while(0)


#define HSV_SWAPPTR(a,b)  do { uint8_t *tmp = (a); (a) = (b); (b) = tmp; } while(0)
#define HSV_POINTER_SWAP(sextant,r,g,b) \
  do { \
    if((sextant) & 2) { \
      HSV_SWAPPTR((r), (b)); \
    } \
    if((sextant) & 4) { \
      HSV_SWAPPTR((g), (b)); \
    } \
    if(!((sextant) & 6)) { \
      if(!((sextant) & 1)) { \
        HSV_SWAPPTR((r), (g)); \
      } \
    } else { \
      if((sextant) & 1) { \
        HSV_SWAPPTR((r), (g)); \
      } \
    } \
  } while(0)

void fast_hsv2rgb_32bit(uint16_t h, uint8_t s, uint8_t v, uint8_t *r, uint8_t *g , uint8_t *b) {
  HSV_MONOCHROMATIC_TEST(s, v, r, g, b);  // Exit with grayscale if s == 0

  uint8_t sextant = h >> 8;

  HSV_POINTER_SWAP(sextant, r, g, b); // Swap pointers depending which sextant we are in

  *g = v;   // Top level

  // Perform actual calculations

  /*
   * Bottom level: v * (1.0 - s)
   * --> (v * (255 - s) + error_corr + 1) / 256
   */
  uint16_t ww;    // Intermediate result
  ww = v * (255 - s); // We don't use ~s to prevent size-promotion side effects
  ww += 1;    // Error correction
  ww += ww >> 8;    // Error correction
  *b = ww >> 8;

  uint8_t h_fraction = h & 0xff;  // 0...255
  uint32_t d;     // Intermediate result

  if(!(sextant & 1)) {
    // *r = ...slope_up...;
    d = v * (uint32_t)((255 << 8) - (uint16_t)(s * (256 - h_fraction)));
    d += d >> 8;  // Error correction
    d += v;   // Error correction
    *r = d >> 16;
  } else {
    // *r = ...slope_down...;
    d = v * (uint32_t)((255 << 8) - (uint16_t)(s * h_fraction));
    d += d >> 8;  // Error correction
    d += v;   // Error correction
    *r = d >> 16;
  }
  
}
