// https://github.com/Makuna/NeoPixelBus
// https://github.com/Makuna/NeoPixelBus/wiki/NeoPixelBus-object-API
// https://github.com/Makuna/NeoPixelBus/wiki/HslColor-object-API

#include <NeoPixelBus.h>
#include <ArduinoJson.h>
#include <EEPROM.h>

#define MAX_PIXELS 300
//#define MAX_PIXELS 300

//#define MAX_BRIGHTNESS 128
//#define MAX_BRIGHTNESS 255
#define MAX_BRIGHTNESS 5

#define ESP
#define VERSION "0.0.4"

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

uint16_t PixelCount;
uint16_t maxchars;
uint16_t frame;
uint32_t micros_start = micros(), micros_end, micros_diff;
uint8_t mode;
uint8_t brightness;
float speed = 2;
float periods = 2;
uint8_t fading = 1;

#define MODE_OFF 0
#define MODE_RAINBOW 1
#define MODE_FILL 2
#define MODE_BINARY 3
#define BIGGEST_MODE_NUMBER 3

struct Effect {
  uint16_t pixels;
  uint8_t mode;
  uint8_t brightness;
  float speed;
  float periods;
  uint8_t fading;
};

void ensureVariableSanity() {
  if (PixelCount > MAX_PIXELS) PixelCount = MAX_PIXELS;
  if (PixelCount < 4) PixelCount = MAX_PIXELS;
  if (mode > BIGGEST_MODE_NUMBER) mode = 1;
  if (brightness == 0) brightness = MAX_BRIGHTNESS;
  maxchars = PixelCount*3;
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
            if(payload[0] == '#') {
                // we get RGB data
                // decode rgb data
                uint32_t rgb = (uint32_t) strtol((const char *) &payload[1], NULL, 16);
                /*analogWrite(LED_RED,    ((rgb >> 16) & 0xFF));
                analogWrite(LED_GREEN,  ((rgb >> 8) & 0xFF));
                analogWrite(LED_BLUE,   ((rgb >> 0) & 0xFF));*/
            }
            handleMessage(payload);
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

  EEPROM.get(0, PixelCount);
  EEPROM.get(2, mode);
  EEPROM.get(3, brightness);
  ensureVariableSanity();
  
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
  fading = 1;

  Serial.print("PixelCount:"); Serial.println(PixelCount);
  Serial.print("mode:"); Serial.println(mode);
  Serial.print("brightness:"); Serial.println(brightness);
  FREERAM_PRINT;

  #ifdef ESP
  IAS.begin('L'); // Optional parameter: What to do with EEPROM on First boot of the app? 'F' Fully erase | 'P' Partial erase(default) | 'L' Leave intact
  IAS.setCallHome(true); // Set to true to enable calling home frequently (disabled by default)
  IAS.setCallHomeInterval(3600*3); // Call home interval in seconds, use 60s only for development. Please change it to at least 2 hours in production
  // start webSocket server
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  #endif
  
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
  
  if (mode == MODE_RAINBOW) {
    uint16_t hue_moving = (((float)(frame))*speed)/((float)PixelCount) * HSV_HUE_MAX;
    float hue_mul = HSV_HUE_MAX/(((float)PixelCount)/periods);
    uint8_t *ptr = strip.Pixels();
    ///uint8_t r,g,b;
    for (uint16_t i = 0; i < PixelCount; i++) {
      uint16_t hue_temp = ((float)i)*hue_mul;
      uint16_t hue = (hue_temp + hue_moving) % HSV_HUE_MAX;
      fast_hsv2rgb_32bit(hue, 255, value, ptr++, ptr++, ptr++);
    }
    //strip.RotateLeft(1); delay(10);
    strip.Dirty();
    strip.Show();
  }

  if (mode == MODE_BINARY || mode == MODE_OFF) {
    //delay(25);
    delay(4);
  }
  
  pollSerial();

  frame++;
  if (frame == (PixelCount/speed)) {
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
    value = brightness;
  }
  if (value >= brightness) {
    value = brightness;
    fading = 0;
  }
  return value;
}

#define IF(a) if (root.containsKey(a))
#define IF(a,b) if (root.containsKey(a)) if (!strcmp(root[a],b))

void pollSerial() {
  if (!Serial.available()) return;
  if (Serial.peek() == 'b') {
    //Serial.println("BINARY");
    //Serial.println(Serial.read());
    Serial.read(); //throw 'b' away
    mode = MODE_BINARY;
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
    /*StaticJsonBuffer<300> jsonBuffer; //ArduinoJson 5
    JsonObject& root = jsonBuffer.parseObject(ptr);*/
    DynamicJsonBuffer jsonBuffer(255); //ArduinoJson5
    JsonObject& root = jsonBuffer.parseObject(Serial);
    // Test if parsing succeeds.
    if (!root.success()) {
      Serial.println("parseObject() failed");
      return;
    }
    if (root["mode"]) { Serial.print("Got mode:\""); Serial.print((const char*)root["mode"]); Serial.println("\""); }
    if (root["pixels"]) {
      PixelCount = root["pixels"];
      Serial.println("PIXELS!");
    }
    IF("mode", "off") {
      frame = 0;
      mode = MODE_OFF;
      Serial.println("OFFMODE!");
      strip.ClearTo(RgbColor(0));
      strip.Show();
    }
    IF("mode", "rainbow") {
      frame = 0;
      mode = MODE_RAINBOW;
      Serial.println("RAINBOWMODE!");
    }
    IF("mode", "fill") {
      mode = MODE_FILL;
      uint8_t r = root["r"], g = root["g"], b = root["b"];
      strip.ClearTo(RgbColor(r,g,b));
      strip.Show();
      Serial.println("FILLMODE!");
    }
    if (root["a"]) { //for Node-RED color picker, which sends r, g, b, a
      mode = MODE_FILL;
      uint8_t r = root["r"], g = root["g"], b = root["b"];
      RgbColor gamma = colorGamma.Correct(RgbColor(r, g, b));
      //strip.ClearTo(RgbColor(r,g,b));
      strip.ClearTo(gamma);
      strip.Show();
      Serial.println("FILLMODE-A!");
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
    IF("fading", "true") fading = 1; else fading = 0;
    IF("save", "true") {
      EEPROM.put(0, PixelCount);
      EEPROM.put(2, mode);
      EEPROM.put(3, brightness);
      Serial.println("SAVE!");
    }
    ensureVariableSanity();
    Serial.print("PixelCount:"); Serial.println(PixelCount);
    Serial.print("mode:"); Serial.println(mode);
    Serial.print("brightness:"); Serial.println(brightness);
    Serial.print("frame:"); Serial.println(frame);
    FREERAM_PRINT;
  }

}

void handleMessage(uint8_t *ptr) {
    StaticJsonBuffer<300> jsonBuffer; //ArduinoJson 5
    //JsonObject& root = jsonBuffer.parseObject(ptr);
    //DynamicJsonBuffer jsonBuffer(255); //ArduinoJson5
    JsonObject& root = jsonBuffer.parseObject(ptr);
    //JsonObject& root = jsonBuffer.parseObject(Serial);
    // Test if parsing succeeds.
    if (!root.success()) {
      Serial.println("parseObject() failed");
      return;
    }
    if (root["mode"]) { Serial.print("Got mode:\""); Serial.print((const char*)root["mode"]); Serial.println("\""); }
    if (root["pixels"]) {
      PixelCount = root["pixels"];
      Serial.println("PIXELS!");
    }
    IF("mode", "off") {
      frame = 0;
      mode = MODE_OFF;
      Serial.println("OFFMODE!");
      clearStrip();
    }
    IF("mode", "rainbow") {
      frame = 0;
      mode = MODE_RAINBOW;
      Serial.println("RAINBOWMODE!");
    }
    IF("mode", "fill") {
      mode = MODE_FILL;
      uint8_t r = root["r"], g = root["g"], b = root["b"];
      strip.ClearTo(RgbColor(r,g,b));
      strip.Show();
      Serial.println("FILLMODE!");
    }
    if (root["a"]) { //for Node-RED color picker, which sends r, g, b, a
      mode = MODE_FILL;
      uint8_t r = root["r"], g = root["g"], b = root["b"];
      RgbColor gamma = colorGamma.Correct(RgbColor(r, g, b));
      //strip.ClearTo(RgbColor(r,g,b));
      strip.ClearTo(gamma);
      strip.Show();
      Serial.println("FILLMODE-A!");
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
    IF("fading", "true") fading = 1; else fading = 0;
    IF("save", "true") {
      EEPROM.put(0, PixelCount);
      EEPROM.put(2, mode);
      EEPROM.put(3, brightness);
      Serial.println("SAVE!");
    }
    ensureVariableSanity();
    Serial.print("PixelCount:"); Serial.println(PixelCount);
    Serial.print("mode:"); Serial.println(mode);
    Serial.print("brightness:"); Serial.println(brightness);
    Serial.print("frame:"); Serial.println(frame);
    FREERAM_PRINT;
}

void clearStrip() {
  strip.ClearTo(RgbColor(0));
  strip.Show();
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
