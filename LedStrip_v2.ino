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
}

#define HSV_HUE_SEXTANT    256
#define HSV_HUE_STEPS   (6 * HSV_HUE_SEXTANT)

#define HSV_HUE_MIN   0
#define HSV_HUE_MAX   (HSV_HUE_STEPS - 1)
#define HSV_SAT_MIN   0
#define HSV_SAT_MAX   255
#define HSV_VAL_MIN   0
#define HSV_VAL_MAX   255
uint8_t r, g, b;

void loop() {
  //Serial.println("loop begin");
  //uint32_t s,e,t;
  //float brightness = MAX_BRIGHTNESS / 255.0f;
  //#define SPEED 100
  #define SPEED 2
  //float f = ((float)(frame % SPEED)) / SPEED;
  //uint16_t f = frame % 1536;
/*  noInterrupts();
  s = micros();*/
  //float hue_moving_mult = ((float)frame)/359.0f * HSV_HUE_MAX;
  //uint16_t hue_moving = (uint16_t) hue_moving_mult;
  uint16_t hue_moving = (uint16_t) (((float)(frame*SPEED))/359.0f * HSV_HUE_MAX);
  float hue_mul = HSV_HUE_MAX/((float)PixelCount);
  uint8_t *p = strip.Pixels();
  for (uint16_t i = 0; i < PixelCount; i++) {
    //float c = ((float)i) / ((float)PixelCount);
    //c += f;
    //float temp = ((int)c); c -= temp; //take only fractional part
    ////HslColor color = HslColor(c, 1.0f, brightness);
    //uint8_t r, g, b;
    //fast_hsv2rgb_32bit(100, 255, 255, &r, &g, &b);
    uint16_t hue_temp = (uint16_t)(((float)i)*hue_mul);
    uint16_t hue = (hue_temp + hue_moving) % HSV_HUE_MAX;
    fast_hsv2rgb_32bit(hue, 255, MAX_BRIGHTNESS, p++, p++, p++);

    /*uint8_t r, g, b;
    fast_hsv2rgb_32bit(frame, 255, 255, &r, &g, &b);
    //r = 100; g = 0; b = 0;
    RgbColor color = RgbColor(r, g, b);
    //Serial.print("r:"); Serial.print(r); Serial.print(" g:");Serial.print(g); Serial.print(" b:");Serial.println(b);
    strip.SetPixelColor(i, color);*/
  }
  //fast_hsv2rgb_32bit(frame, 255, 255, &r, &g, &b);
  //Serial.print("r:"); Serial.print(r); Serial.print(" g:");Serial.print(g); Serial.print(" b:");Serial.println(b);
/*  e = micros();
  interrupts();
  t = e - s;
  Serial.println(t);

  noInterrupts();
  s = micros();*/
  strip.Dirty();
  strip.Show();
/*  e = micros();
  interrupts();
  t = e - s;
  Serial.println(t);*/
  /*delay(1000);

  Serial.println("Colors R, G, B, W...");

    // set the colors, 
    // if they don't match in order, you need to use NeoGrbFeature feature
    strip.SetPixelColor(0, red);
    strip.SetPixelColor(1, green);
    strip.SetPixelColor(2, blue);
    strip.SetPixelColor(3, white);
    // the following line demonstrates rgbw color support
    // if the NeoPixels are rgbw types the following line will compile
    // if the NeoPixels are anything else, the following line will give an error
    //strip.SetPixelColor(3, RgbwColor(colorSaturation));
    strip.Show();


    delay(1000);

    Serial.println("Off ...");

    // turn off the pixels
    strip.SetPixelColor(0, black);
    strip.SetPixelColor(1, black);
    strip.SetPixelColor(2, black);
    strip.SetPixelColor(3, black);
    strip.Show();*/

  //strip.RotateRight(1);
  //strip.Show();
    
  pollSerial();

  if (frame == 0) {
    micros_end = micros();
    micros_diff = micros_end - micros_start;
    Serial.println(micros_diff);
    FREERAM_PRINT;
    Serial.flush();
    micros_start = micros();
  }
  frame++;
  if (frame == 360) frame = 0;
  
  //delay(500);


}


void pollSerial() {
  if (!Serial.available()) return;
  //uint16_t cnt = Serial.readBytes(data, maxchars);
  uint16_t cnt = Serial.readBytes(strip.Pixels(), maxchars);
  //data[cnt] = '\0';
  Serial.print(cnt); //Serial.print(":"); Serial.println(data);
  strip.Dirty();
  strip.Show();
}




//#define HSV_USE_SEXTANT_TEST  /* Limit the hue to 0...360 degrees */

//void fast_hsv2rgb_8bit(uint16_t h, uint8_t s, uint8_t v, uint8_t *r, uint8_t *g , uint8_t *b);
//void fast_hsv2rgb_32bit(uint16_t h, uint8_t s, uint8_t v, uint8_t *r, uint8_t *g , uint8_t *b);

/*
 * Macros that are common to all implementations
 */
#define HSV_MONOCHROMATIC_TEST(s,v,r,g,b) \
  do { \
    if(!(s)) { \
       *(r) = *(g) = *(b) = (v); \
      return; \
    } \
  } while(0)

/*#ifdef HSV_USE_SEXTANT_TEST
#define HSV_SEXTANT_TEST(sextant) \
  do { \
    if((sextant) > 5) { \
      (sextant) = 5; \
    } \
  } while(0)
#else
#define HSV_SEXTANT_TEST(sextant) do { ; } while(0)
#endif*/

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
