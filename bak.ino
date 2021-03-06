// https://github.com/Makuna/NeoPixelBus
// https://github.com/Makuna/NeoPixelBus/wiki/NeoPixelBus-object-API
// https://github.com/Makuna/NeoPixelBus/wiki/HslColor-object-API

#include <NeoPixelBus.h>
#include <MemoryUsage.h>
#include <ArduinoJson.h>
#include <EEPROM.h>

//#define MAX_PIXELS 600
#define MAX_PIXELS 300

//#define DEBUG

uint16_t PixelCount;
const uint8_t PixelPin = 2;
uint16_t maxchars;
uint16_t frame;
uint32_t micros_start = micros(), micros_end, micros_diff;
uint8_t mode;
uint8_t brightness;
float speed = 2;
float periods = 2;
uint8_t fading = 1;

#define MODE_RAINBOW 1
#define MODE_BINARY 2

//#define MAX_BRIGHTNESS 128
#define MAX_BRIGHTNESS 255
//#define MAX_BRIGHTNESS 10

#ifdef DEBUG
#define DBG_MSG(msg) Serial.println(msg);
#else
#define DBG_MSG(msg)
#endif

//StaticJsonDocument<300> doc;
//DynamicJsonDocument jsonBuffer(300);

//NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod>* strip = NULL;
NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(MAX_PIXELS, PixelPin);
//NeoPixelBus<NeoBrgFeature, Neo800KbpsMethod> strip(MAX_PIXELS, PixelPin);

void setup() {
  //delay(500);
  //Serial.begin(115200);
  Serial.begin(2000000);
  Serial.setTimeout(10);
  #ifdef DEBUG
  while (!Serial) continue;
  #endif

  Serial.println();
  Serial.println("Initializing...");
  Serial.flush();

  strip.Begin();

  Serial.println();
  Serial.println("Running...");
  DBG_MSG("DEBUG!");

  EEPROM.get(0, PixelCount);
  EEPROM.get(2, mode);
  EEPROM.get(3, brightness);
  if (PixelCount > MAX_PIXELS) PixelCount = MAX_PIXELS;
  if (PixelCount < 4) PixelCount = MAX_PIXELS;
  if (mode > 2 || mode < 1) mode = 1;
  if (brightness == 0) brightness = MAX_BRIGHTNESS;
  maxchars = PixelCount*3;
  
  for(uint16_t i = 0; i < PixelCount; i++) {
    float c = ((float)i) / ((float)PixelCount);
    //HslColor color = HslColor(c, 1.0f, ((float)brightness)/2.0f/MAX_BRIGHTNESS);
    //HslColor color = HslColor(c, 1.0f, MAX_BRIGHTNESS / 255.0f);
    HslColor color = HslColor(c, 1.0f, 0.5f);
    strip.SetPixelColor(i, color);
  }
  strip.Show();
  
  frame = 0;
  fading = 1;

  Serial.print("PixelCount:"); Serial.println(PixelCount);
  Serial.print("mode:"); Serial.println(mode);
  Serial.print("brightness:"); Serial.println(brightness);
  FREERAM_PRINT;
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

  uint16_t value;
  if (fading) {
    value = pow((double)frame, 3.0) / 8000.0;
  } else {
    value = brightness;
  }
  if (value > brightness) {
    value = brightness;
    fading = 0;
  }
  
  if (mode == MODE_RAINBOW) {
    uint16_t hue_moving = (((float)(frame))*speed)/((float)PixelCount) * HSV_HUE_MAX;
    float hue_mul = HSV_HUE_MAX/(((float)PixelCount)/periods);
    uint8_t *ptr = strip.Pixels();
    uint8_t r,g,b;
    for (uint16_t i = 0; i < PixelCount; i++) {
    //for (uint16_t i = 0; i < 10; i++) {
      uint16_t hue_temp = ((float)i)*hue_mul;
      uint16_t hue = (hue_temp + hue_moving) % HSV_HUE_MAX;
      //HslColor color = HslColor(c, 1.0f, MAX_BRIGHTNESS / 255.0f);
      //RgbColor rgb = RgbColor(HslColor(hue/1535.0f, 1.0f, MAX_BRIGHTNESS / 255.0f));
      //ptr[i*3] = rgb.R; ptr[i*3+1] = rgb.G; ptr[i*3+2] = rgb.B;
      fast_hsv2rgb_32bit(hue, 255, value, ptr++, ptr++, ptr++);
      //fast_hsv2rgb_32bit(hue, 255, value, p, p, p);
      //fast_hsv2rgb_32bit(hue, 255, value, ptr[i*3], ptr[i*3+1], ptr[i*3+2]);
      //fast_hsv2rgb_32bit(hue, 255, 128, r, g, b);
      //fast_hsv2rgb_32bit(hue, 255, value, &r, &g, &b);
      //r = i;
      //ptr[i*3] = r; ptr[i*3+1] = g; ptr[i*3+2] = b;
      //fast_hsv2rgb_32bit(hue, 255, 128, &ptr[i*3], &ptr[i*3+1], &ptr[i*3+2]);
      //ptr[i*3] = (i + frame) > 1; ptr[i*3+1] = i > 1; ptr[i*3+2] = i > 1;
      //ptr[i*3] = frame; ptr[i*3+1] = i; ptr[i*3+2] = 0;
      //fast_hsv2rgb_32bit(hue, 255, value, ptr[0], ptr[1], ptr[2]);
      //ptr+=3;
      //fast_hsv2rgb_32bit(hue, 255, 128, p++, p++, p++);
      //if (i % 50 == 0) delay(10);
      //delay(10);
    }
    //strip.RotateLeft(1);
    //delay(10);
    /*Serial.println(r);
    Serial.println(g);
    Serial.println(b);*/
    strip.Dirty();
    strip.Show();
  }

  if (mode == MODE_BINARY) {
    //delay(25);
    delay(4);
  }
  
  pollSerial();

  frame++;
  if (frame == (PixelCount/speed)) {
    frame = 0;
    Serial.println("frame0");
  }
  
  //delay(20);
  //delay(500);


}

#define IF(a,b) if (!strcmp(root[a],b))

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
    /*DynamicJsonDocument doc;
    DeserializationError error = deserializeJson(doc, ptr, cnt);
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.c_str());
      return;
    }
    JsonObject root = doc.as<JsonObject>();*/
    StaticJsonBuffer<300> jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(ptr);
    // Test if parsing succeeds.
    if (!root.success()) {
      Serial.println("parseObject() failed");
      return;
    }
    /*if (root["mode"]) { Serial.print("Got mode:\""); Serial.print((const char*)root["mode"]); Serial.println("\""); }
    if (root["pixels"]) {
      PixelCount = root["pixels"];
      if (PixelCount > MAX_PIXELS) PixelCount = MAX_PIXELS;
      if (PixelCount < 4) PixelCount = 4;
      maxchars = PixelCount*3;
      Serial.println("PIXELS!");
    }
    IF("mode", "rainbow") {
      frame = 0;
      mode = MODE_RAINBOW;
      Serial.println("RAINBOWMODE!");
    }
    IF("mode", "binary") {
      mode = MODE_BINARY;
      Serial.println("BINARYMODE!");
    }
    if (root["brightness"]) {
      brightness = root["brightness"];
      Serial.println("BRIGHTNESS!");
    }
    if (root["speed"]) {
      speed = root["speed"];
      Serial.println("SPEED!");
    }
    if (root["periods"]) {
      periods = root["periods"];
      Serial.println("PERIODS!");
    }
    IF("save", "true") {
      EEPROM.put(0, PixelCount);
      EEPROM.put(2, mode);
      EEPROM.put(3, brightness);
      Serial.println("SAVE!");
    }
    IF("fading", "true") fading = 1; else fading = 0;*/
    Serial.print("PixelCount:"); Serial.println(PixelCount);
    Serial.print("mode:"); Serial.println(mode);
    Serial.print("brightness:"); Serial.println(brightness);
    Serial.print("frame:"); Serial.println(frame);
    FREERAM_PRINT;
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
