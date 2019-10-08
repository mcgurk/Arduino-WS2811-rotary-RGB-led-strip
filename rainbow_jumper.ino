// https://github.com/Makuna/NeoPixelBus
// https://github.com/Makuna/NeoPixelBus/wiki/NeoPixelBus-object-API
// https://github.com/Makuna/NeoPixelBus/wiki/HslColor-object-API

#include <NeoPixelBus.h>

#define PIXELS 300
//#define BRIGHTNESS 127
#define BRIGHTNESS 200
#define VELOCITY 6.0f
#define PERIODS 1.0f
#define MODE 0

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

  
const uint8_t sine_table[] PROGMEM = \
{ 128,131,134,137,140,143,146,149, \
152,155,158,162,165,167,170,173, \
176,179,182,185,188,190,193,196, \
198,201,203,206,208,211,213,215, \
218,220,222,224,226,228,230,232, \
234,235,237,238,240,241,243,244, \
245,246,248,249,250,250,251,252, \
253,253,254,254,254,255,255,255, \
255,255,255,255,254,254,254,253, \
253,252,251,250,250,249,248,246, \
245,244,243,241,240,238,237,235, \
234,232,230,228,226,224,222,220, \
218,215,213,211,208,206,203,201, \
198,196,193,190,188,185,182,179, \
176,173,170,167,165,162,158,155, \
152,149,146,143,140,137,134,131, \
128,124,121,118,115,112,109,106, \
103,100,97,93,90,88,85,82, \
79,76,73,70,67,65,62,59, \
57,54,52,49,47,44,42,40, \
37,35,33,31,29,27,25,23, \
21,20,18,17,15,14,12,11, \
10,9,7,6,5,5,4,3, \
2,2,1,1,1,0,0,0, \
0,0,0,0,1,1,1,2, \
2,3,4,5,5,6,7,9, \
10,11,12,14,15,17,18,20, \
21,23,25,27,29,31,33,35, \
37,40,42,44,47,49,52,54, \
57,59,62,65,67,70,73,76, \
79,82,85,88,90,93,97,100, \
103,106,109,112,115,118,121,124 };

const uint8_t PixelPin = 2;
uint16_t frame = 0;
float phase = 0;
uint32_t old_t = 0;
uint8_t brightness = BRIGHTNESS;
uint8_t mode = MODE;

NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(PIXELS, PixelPin);

void setup() {
  //Serial.begin(115200);
  
  pinMode(7, OUTPUT);
  pinMode(6, INPUT_PULLUP);
  pinMode(8, INPUT_PULLUP);
  digitalWrite(7, LOW);

  pinMode(14, OUTPUT);
  pinMode(15, INPUT_PULLUP);
  pinMode(16, INPUT_PULLUP);
  digitalWrite(14, LOW);
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
  float hue_mul = HSV_HUE_MAX/(PIXELS/PERIODS);
  uint8_t *ptr;
  ptr = strip.Pixels();

  if (mode == 1) {

    //uint32_t p1 = (uint32_t)(phase*7000);
    //uint32_t p2 = (uint32_t)(phase*10000);
    uint32_t p1 = (uint32_t)(phase*7000); //hue
    uint32_t p2 = (uint32_t)(phase*15000); //value
      
    for (uint32_t i = 0; i < PIXELS; i++) {
      #define A 60.0/360.0*HSV_HUE_MAX/255.0
      #define C 180.0/360.0*HSV_HUE_MAX
      //uint8_t x1 = -i*255*3/PIXELS+p1;
      //uint8_t x2 = i*255*4/PIXELS+p2;
      uint8_t x1 = -i*3+p1;
      uint8_t x2 = i*4+p2;
      uint16_t hue = pgm_read_byte_near(sine_table + x1)*A+C;
      //uint16_t hue = 0;
      //hue = HSV_HUE_MAX/3*2;
      uint8_t v = pgm_read_byte_near(sine_table + x2)*brightness/255.0;
      //fast_hsv2rgb_32bit(hue, 255, v/1.5+50.0, ptr++, ptr++, ptr++);
      fast_hsv2rgb_32bit(hue, 255, v+50, ptr++, ptr++, ptr++);
    }

    //for (int i = 0; i < 256; i++) Serial.println(sine_table[i]);
    //for (int i = 0; i < 256; i++) Serial.println(i);
    //for (int i = 0; i < 256; i++) Serial.println(pgm_read_byte_near(sine_table + i));
  }

  if (mode == 0) {
    //uint16_t hue_moving = phase * HSV_HUE_MAX;
    //float hue_mul = HSV_HUE_MAX/(PIXELS/PERIODS);
    //uint8_t *ptr = strip.Pixels();
    for (uint16_t i = 0; i < PIXELS; i++) {
      uint16_t hue_temp = ((float)i)*hue_mul;
      uint16_t hue = (hue_temp + hue_moving) % HSV_HUE_MAX;
      fast_hsv2rgb_32bit(hue, 255, brightness, ptr++, ptr++, ptr++);
    }
  }
  
  #ifdef GAMMA
  ptr = strip.Pixels(); 
  for (uint16_t i = 0; i < PIXELS*3; i++) {
    *ptr = pgm_read_byte_near(gamma_table + *ptr);
    ptr++;
  }
  #endif

  strip.Dirty();
  strip.Show();

  frame++;
  phase += VELOCITY/((float)(micros()-old_t));
  old_t = micros();
  phase = fmod(phase, 1.0f);

  uint8_t a = digitalRead(6); uint8_t b = digitalRead(8);
  if(a && b) brightness = 100; // no jumper
  if(!a && b) brightness = 150; // 6-7
  if(a && !b) brightness = BRIGHTNESS; // 7-8
  //Serial.print(a); Serial.print(" "); Serial.print(b); Serial.print(" "); Serial.println(brightness); delay(100);
  //delay(20);
  uint8_t c = digitalRead(15); uint8_t d = digitalRead(16);
  if(c && d) mode = MODE; // no jumper
  if(!c && d) mode = MODE; // 14-15
  if(c && !d) mode = 1; // 14-16

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

//void fast_hsv2rgb_32bit(uint16_t h, uint8_t s, uint8_t v, uint8_t *r, uint8_t *g , uint8_t *b) {
void fast_hsv2rgb_32bit(uint16_t h, uint8_t s, uint8_t v, uint8_t *b, uint8_t *r , uint8_t *g) {
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
