// https://github.com/Makuna/NeoPixelBus
// https://github.com/Makuna/NeoPixelBus/wiki/NeoPixelBus-object-API
// https://github.com/Makuna/NeoPixelBus/wiki/HslColor-object-API

#include <NeoPixelBus.h>
#include <MemoryUsage.h>

#define MAX_PIXELS 600

//uint16_t PixelCount = 10;
uint16_t PixelCount = 150;
//uint16_t PixelCount = MAX_PIXELS;
const uint8_t PixelPin = 2;
uint16_t maxchars = PixelCount*3;
uint16_t frame = 0;
uint32_t micros_start = micros(), micros_end, micros_diff;
uint8_t mode;
#define MODE_RAINBOW 1
#define MODE_BINARY 2

#define MAX_BRIGHTNESS 128

//NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod>* strip = NULL;
NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(MAX_PIXELS, PixelPin);

void setup() {
  Serial.begin(115200);
  //while (!Serial); // wait for serial attach

  Serial.println();
  Serial.println("Initializing...");
  Serial.flush();

  //strip = new NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod>(PixelCount, PixelPin); // and recreate with new count
  strip.Begin();
  //strip.Show();

  Serial.println();
  Serial.println("Running...");

  /*for(uint16_t i = 0; i < PixelCount; i++) {
    float c = ((float)i) / ((float)PixelCount);
    //HslColor color = HslColor(c, 1.0f, ((float)brightness)/2.0f/MAX_BRIGHTNESS);
    HslColor color = HslColor(c, 1.0f, MAX_BRIGHTNESS / 255.0f);
    strip.SetPixelColor(i, color);
  }
  strip.Show();*/
  if (PixelCount > MAX_PIXELS) PixelCount = MAX_PIXELS;
  mode = MODE_RAINBOW;
}

#define HSV_HUE_SEXTANT    256
#define HSV_HUE_STEPS   (6 * HSV_HUE_SEXTANT)

#define HSV_HUE_MIN   0
#define HSV_HUE_MAX   (HSV_HUE_STEPS - 1)
#define HSV_SAT_MIN   0
#define HSV_SAT_MAX   255
#define HSV_VAL_MIN   0
#define HSV_VAL_MAX   255
//uint8_t r, g, b;
#define SPEED 2

void loop() {

  if (mode == MODE_RAINBOW) {
    uint16_t hue_moving = (uint16_t) (((float)(frame*SPEED))/359.0f * HSV_HUE_MAX);
    float hue_mul = HSV_HUE_MAX/((float)PixelCount);
    uint8_t *p = strip.Pixels();
    for (uint16_t i = 0; i < PixelCount; i++) {
      uint16_t hue_temp = (uint16_t)(((float)i)*hue_mul);
      uint16_t hue = (hue_temp + hue_moving) % HSV_HUE_MAX;
      fast_hsv2rgb_32bit(hue, 255, MAX_BRIGHTNESS, p++, p++, p++);
    }
    strip.Dirty();
    strip.Show();
  }

  if (mode == MODE_BINARY) {
    delay(25); 
  }
  
  pollSerial();

  if (frame == 0) {
    micros_end = micros();
    micros_diff = micros_end - micros_start;
    //Serial.println(micros_diff);
    FREERAM_PRINT;
    Serial.print("mode:"); Serial.println(mode);
    Serial.flush();
    micros_start = micros();
  }
  frame++;
  if (frame == 360) frame = 0;
  
  //delay(500);


}


void pollSerial() {
  if (!Serial.available()) return;
  char *ptr = strip.Pixels();
  uint16_t cnt = Serial.readBytes(ptr, maxchars);
  if (cnt == maxchars) {
    mode = MODE_BINARY;
    strip.Dirty();
    strip.Show();
  } else {
    ptr[cnt] = '\0';
    Serial.print(cnt); Serial.print(":"); Serial.println(ptr);
    if (ptr[0] == 'r') mode = MODE_RAINBOW;
    if (ptr[0] == 'b') mode = MODE_BINARY;
    Serial.print("mode:"); Serial.println(mode);
    Serial.print("frame:"); Serial.println(frame);
  }
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

  //HSV_SEXTANT_TEST(sextant);    // Optional: Limit hue sextants to defined space

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
