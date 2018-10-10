// https://github.com/Makuna/NeoPixelBus
// https://github.com/Makuna/NeoPixelBus/wiki/NeoPixelBus-object-API
// https://github.com/Makuna/NeoPixelBus/wiki/HslColor-object-API

#include <NeoPixelBus.h>
#include <ArduinoJson.h>
#include <EEPROM.h>

#define MAX_PIXELS 300
//#define MAX_PIXELS 300

//#define MAX_BRIGHTNESS 128
#define MAX_BRIGHTNESS 255
//#define MAX_BRIGHTNESS 5

#define ESP
#define VERSION "0.0.8"

//#define DEBUG

#ifdef ESP
#define FREERAM_PRINT
#else
#include <MemoryUsage.h>
#endif

#ifdef ESP
#define COMPDATE __DATE__ __TIME__
#define MODEBUTTON 0                                        // Button pin on the esp for selecting modes. D3 for the Wemos!
#include <IOTAppStory.h>                                    // IotAppStory.com library
IOTAppStory IAS(COMPDATE, MODEBUTTON);                      // Initialize IOTAppStory
#include <WebSocketsServer.h>
WebSocketsServer webSocket = WebSocketsServer(81);
#endif

const uint8_t PixelPin = 2; //doesn't matter on ESP8266. Allways GPIO3/RX

uint16_t maxchars;
uint16_t frame;
uint32_t micros_start = micros(), micros_end, micros_diff;
uint8_t fading = 1;

#define MODE_OFF 0
#define MODE_RAINBOW 1
#define MODE_FILL 2
#define MODE_BINARY 3
#define MODE_VUMETER 4
#define BIGGEST_MODE_NUMBER 4

struct Effect {
  uint16_t pixels;
  uint8_t mode;
  uint8_t brightness;
  float speed;
  float periods;
  uint8_t fading;
  RgbColor color;
} effect;

void ensureEffectSanity() {
  if (effect.pixels > MAX_PIXELS) effect.pixels = MAX_PIXELS;
  if (effect.pixels < 4) effect.pixels = 4;
  if (effect.mode > BIGGEST_MODE_NUMBER) effect.mode = 1;
  if (effect.brightness == 0) effect.brightness = MAX_BRIGHTNESS;
  if (effect.periods == 0) effect.periods = 1;
  if (effect.speed == 0) effect.speed = 1;
  if (effect.fading != 1) effect.fading = 0;
  maxchars = effect.pixels*3;
}

#ifdef DEBUG
#define DBG_MSG(msg) Serial.println(msg);
#else
#define DBG_MSG(msg)
#endif

#ifdef ESP
NeoPixelBus<NeoGrbFeature, NeoEsp8266Uart800KbpsMethod> strip(MAX_PIXELS, PixelPin);
#else
NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(MAX_PIXELS, PixelPin);
#endif
//NeoGamma<NeoGammaTableMethod> colorGamma;
NeoGamma<NeoGammaEquationMethod> colorGamma;

#ifdef ESP
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
    switch(type) {
        case WStype_DISCONNECTED:
            Serial.printf("[%u] Disconnected!\n", num);
            break;
        case WStype_CONNECTED: {
            IPAddress ip = webSocket.remoteIP(num);
            Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
            // send message to client
            webSocket.sendTXT(num, "Connected");
        }
            break;
        case WStype_TEXT:
            Serial.printf("[%u] get Text: %s\n", num, payload);
            /*if(payload[0] == '#') {
                // we get RGB data
                // decode rgb data
                uint32_t rgb = (uint32_t) strtol((const char *) &payload[1], NULL, 16);
                analogWrite(LED_RED,    ((rgb >> 16) & 0xFF));
                analogWrite(LED_GREEN,  ((rgb >> 8) & 0xFF));
                analogWrite(LED_BLUE,   ((rgb >> 0) & 0xFF));
            }*/
            //StaticJsonBuffer<300> jsonBuffer; //ArduinoJson 5
            DynamicJsonBuffer jsonBuffer(255); //ArduinoJson 5
            JsonObject& root = jsonBuffer.parseObject(payload);
            parseStripParameters(root);
            break;
    }
}
#endif
void setup() {
  #ifdef ESP
  IAS.preSetDeviceName("Ledstrip-wemos"); // preset deviceName this is also your MDNS responder: http://iasblink.local
  IAS.preSetAppName(F("Ledstrip")); // preset appName
  IAS.preSetAppVersion(F(VERSION)); // preset appVersion
  IAS.preSetAutoUpdate(true); // automaticUpdate (true, false)
  Serial.begin(115200);
  #else
  Serial.begin(2000000);
  #endif
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

  #ifdef ESP
  EEPROM.begin(1536);
  #endif
  EEPROM.get(1024, effect);
  #ifdef ESP
  EEPROM.end();
  #endif
  ensureEffectSanity();
  
  /*for(uint16_t i = 0; i < PixelCount; i++) {
    float c = ((float)i) / ((float)PixelCount);
    //HslColor color = HslColor(c, 1.0f, ((float)brightness)/2.0f/MAX_BRIGHTNESS);
    //HslColor color = HslColor(c, 1.0f, MAX_BRIGHTNESS / 255.0f);
    HslColor color = HslColor(c, 1.0f, 0.5f);
    strip.SetPixelColor(i, color);
  }
  strip.Show();*/
  
  clearStrip();
  
  frame = 0;
  fading = effect.fading;

  #ifdef ESP
  IAS.begin('L'); // Optional parameter: What to do with EEPROM on First boot of the app? 'F' Fully erase | 'P' Partial erase(default) | 'L' Leave intact
  IAS.setCallHome(true); // Set to true to enable calling home frequently (disabled by default)
  IAS.setCallHomeInterval(3600*3); // Call home interval in seconds, use 60s only for development. Please change it to at least 2 hours in production
  // start webSocket server
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  #endif

  printStatus();
  
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
  #ifdef ESP
  IAS.loop();  // this routine handles the calling home on the configured itnerval as well as reaction of the Flash button. If short press: update of skethc, long press: Configuration
  webSocket.loop();
  #endif

  uint16_t value = handleFading();
  
  if (effect.mode == MODE_RAINBOW) {
    uint16_t hue_moving = (((float)(frame))*effect.speed)/((float)effect.pixels) * HSV_HUE_MAX;
    float hue_mul = HSV_HUE_MAX/(((float)effect.pixels)/effect.periods);
    uint8_t *ptr = strip.Pixels();
    for (uint16_t i = 0; i < effect.pixels; i++) {
      uint16_t hue_temp = ((float)i)*hue_mul;
      uint16_t hue = (hue_temp + hue_moving) % HSV_HUE_MAX;
      fast_hsv2rgb_32bit(hue, 255, value, ptr++, ptr++, ptr++);
    }
    //strip.RotateLeft(1); delay(10);
    strip.Dirty();
    strip.Show();
  }

  if (effect.mode == MODE_BINARY || effect.mode == MODE_OFF) {
    //delay(25);
    delay(4);
  }

  if (effect.mode == MODE_VUMETER) {
    #define SENSORPIN A0
    int sensorValue = analogRead(SENSORPIN);
    strip.ClearTo(RgbColor(0));
    strip.ClearTo(effect.color, 0, sensorValue * effect.pixels / 1023);
    strip.Show();
    delay(4);
  }
  
  pollSerial();

  frame++;
  if (frame == (effect.pixels/effect.speed)) {
    frame = 0;
    fading = 0;
    //Serial.println("frame0");
  }
  
  //delay(20);
}


uint16_t handleFading() {
  uint16_t value;
  if (fading) {
    int16_t f = frame*2; if (f > 255) f = 255;
    RgbColor gamma = colorGamma.Correct(RgbColor(f, 0, 0));
    value = gamma.R;
    //value = pow((double)frame, 3.0) / 8000.0;
  } else {
    value = effect.brightness;
  }
  if (value >= effect.brightness) {
    value = effect.brightness;
    fading = 0;
  }
  return value;
}

// https://arduinojson.org/v5/api/jsonobject/containskey/
#define IFKEY(a) if (root.containsKey(a))
#define IF(a,b) IFKEY(a) if (!strcmp(root[a],b))
//#define IF(a,b) if (root.containsKey(a)) if (!strcmp(root[a],b))
//#define IFKEY(a) if (root[a]) //doesn't work!?
//#define IF(a,b) if (root[a]) if (!strcmp(root[a],b))

void pollSerial() {
  if (!Serial.available()) return;
  if (Serial.peek() == 'b') {
    //Serial.println("BINARY");
    //Serial.println(Serial.read());
    Serial.read(); //throw 'b' away
    effect.mode = MODE_BINARY;
    char *ptr = (char*)strip.Pixels();
    uint16_t cnt = Serial.readBytes(ptr, maxchars);
    strip.Dirty();
    strip.Show();
  } else {
    //Serial.println("SOMETHINGELSE");
    //ptr[cnt] = '\0';
    /*DynamicJsonDocument doc; //ArduinoJson 6
    DeserializationError error = deserializeJson(doc, ptr, cnt);
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.c_str());
      return;
    }
    JsonObject root = doc.as<JsonObject>();*/
    //StaticJsonBuffer<300> jsonBuffer; //ArduinoJson 5
    DynamicJsonBuffer jsonBuffer(255); //ArduinoJson 5
    JsonObject& root = jsonBuffer.parseObject(Serial);
    parseStripParameters(root);
  }

}

void parseStripParameters(JsonObject& root) {
    // Test if parsing succeeds.
    if (!root.success()) {
      Serial.println("parseObject() failed");
      return;
    }
    IFKEY("mode") { 
      Serial.print("Got mode:\""); Serial.print((const char*)root["mode"]); Serial.println("\""); 
    }
    IFKEY("pixels") {
      effect.pixels = root["pixels"];
      Serial.println("PIXELS!");
    }
    IF("mode", "off") {
      frame = 0;
      effect.mode = MODE_OFF;
      Serial.println("OFFMODE!");
      clearStrip();
    }
    IF("mode", "rainbow") {
      frame = 0;
      effect.mode = MODE_RAINBOW;
      Serial.println("RAINBOWMODE!");
    }
    IF("mode", "vumeter") {
      frame = 0;
      effect.mode = MODE_VUMETER;
      Serial.println("VUMETERMODE!");
    }
    IF("mode", "fill") {
      effect.mode = MODE_FILL;
      uint8_t r = root["r"], g = root["g"], b = root["b"];
      effect.color = RgbColor(r,g,b);
      strip.ClearTo(effect.color);
      strip.Show();
      Serial.println("FILLMODE!");
    }
    IFKEY("r") IFKEY("g") IFKEY("b") {
      effect.mode = MODE_FILL;
      uint8_t r = root["r"], g = root["g"], b = root["b"];
      effect.color = colorGamma.Correct(RgbColor(r, g, b));
      //strip.ClearTo(RgbColor(r,g,b));
      strip.ClearTo(effect.color);
      strip.Show();
      Serial.println("FILLMODE-RGB-GAMMA!");
    }
    IFKEY("h") IFKEY("s") IFKEY("l") {
      effect.mode = MODE_FILL;
      float h = root["h"], s = root["s"], l = root["l"];
      RgbColor c = HslColor(h/360.0f, s, l/2.0f);
      effect.color = colorGamma.Correct(c);
      strip.ClearTo(effect.color);
      strip.Show();
      Serial.println("FILLMODE-HSL-GAMMA!");
    }
    IF("mode", "binary") {
      effect.mode = MODE_BINARY;
      Serial.println("BINARYMODE!");
    }
    IFKEY("brightness") {
      effect.brightness = root["brightness"];
      Serial.println("BRIGHTNESS!");
    }
    IFKEY("speed") {
      effect.speed = root["speed"];
      Serial.println("SPEED!");
    }
    IFKEY("periods") {
      effect.periods = root["periods"];
      Serial.println("PERIODS!");
    }
    IF("fading", "true") {
      effect.fading = 1;
      fading = 1;
    } else {
      effect.fading = 0;
      fading = 0;
    }
    IF("save", "true") {
      #ifdef ESP
      EEPROM.begin(1536);
      #endif
      EEPROM.put(1024, effect);
      #ifdef ESP
      EEPROM.end();
      #endif
      Serial.println("SAVE!");
    }
    IFKEY("load") {
      #ifdef ESP
      EEPROM.begin(1536);
      #endif
      EEPROM.get(1024, effect);
      #ifdef ESP
      EEPROM.end();
      #endif
    }
    ensureEffectSanity();
    printStatus();
}

void clearStrip() {
  strip.ClearTo(RgbColor(0));
  strip.Show();
}

void printStatus() {
  Serial.print("PixelCount:"); Serial.println(effect.pixels);
  Serial.print("mode:"); Serial.println(effect.mode);
  Serial.print("brightness:"); Serial.println(effect.brightness);
  Serial.print("speed:"); Serial.println(effect.speed);
  Serial.print("periods:"); Serial.println(effect.periods);
  Serial.print("fading:"); Serial.println(effect.fading);
  Serial.print("frame:"); Serial.println(frame);
  FREERAM_PRINT;
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

void fast_hsv2rgb_32bit(uint16_t h, uint8_t s, uint8_t v, uint8_t *g, uint8_t *r , uint8_t *b) {
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
