// https://github.com/Makuna/NeoPixelBus
// https://github.com/Makuna/NeoPixelBus/wiki/NeoPixelBus-object-API
// https://github.com/Makuna/NeoPixelBus/wiki/HslColor-object-API

#include <NeoPixelBus.h>
//#include <ArduinoJson.h>
#include <EEPROM.h>
#include <IRremote.h>

#define recvPin 8
#define GND 9
#define VCC 10
IRrecv irrecv(recvPin);

#define MAX_PIXELS 100
#define MAX_BRIGHTNESS 128

#define DEBUG
#ifdef DEBUG
#define debug(x) Serial.println(x)
#else
#define debug(x)
#endif

#define POWER_BTN 0xFFB04F // power (red)
#define RESET_BTN 0xFFB24D // <-/-> (yellow)
#define SPEED_DOWN_BTN 0xFF28D7 // <-..
#define SPEED_UP_BTN 0xFFF00F // ..->
#define SAVE_BTN 0xFF9867 // IC set
#define WIDE_BTN 0xFFD827 // < | > (green)
#define NARROW_BTN 0xFF8877 // > | < (blue)


const uint8_t PixelPin = 2;

uint16_t maxchars;
uint32_t frame;
//uint32_t micros_start = micros(), micros_end, micros_diff;
uint8_t fading = 1;
uint8_t power = 1;
uint32_t old_t = 0;

//#define MODE_OFF 0
#define MODE_RAINBOW 1
#define MODE_FILL 2
#define MODE_BINARY 3 // get values from serial port as raw binary. prefix raw data with byte 'b'.
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
  float phase;
  uint16_t checksum;
} effect;

void ensureEffectSanity() {
  if (effect.pixels > MAX_PIXELS) effect.pixels = MAX_PIXELS;
  if (effect.pixels < 4) effect.pixels = 4;
  if (effect.mode > BIGGEST_MODE_NUMBER) effect.mode = 1;
  if (effect.brightness == 0) effect.brightness = MAX_BRIGHTNESS;
  if (effect.brightness > MAX_BRIGHTNESS) effect.brightness = MAX_BRIGHTNESS;
  if (effect.periods < 0) effect.periods = 0;
  //if (effect.speed == 0) effect.speed = 1;
  if (effect.fading != 1) effect.fading = 0;
  maxchars = effect.pixels*3;
}

#ifdef DEBUG
#define DBG_MSG(msg) Serial.println(msg);
#else
#define DBG_MSG(msg)
#endif

NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(MAX_PIXELS, PixelPin);
NeoGamma<NeoGammaEquationMethod> colorGamma;


void setup() {
  pinMode(GND, OUTPUT); digitalWrite(GND, LOW);
  pinMode(VCC, OUTPUT); digitalWrite(VCC, HIGH);
  Serial.begin(115200);
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

  EEPROM.get(1024, effect);
  ensureEffectSanity();
  
  clearStrip();
  
  frame = 0;
  fading = effect.fading;

  printStatus();
  
  irrecv.enableIRIn();  // Start the receiver

  for(uint16_t i = 0; i < MAX_PIXELS; i++) {
    float c = ((float)i) / ((float)MAX_PIXELS);
    HslColor color = HslColor(c, 1.0f, 0.5f);
    strip.SetPixelColor(i, color);
  }
  strip.Show();
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
  //uint16_t value = handleFading();

  if (effect.mode == MODE_RAINBOW && power) {
    uint16_t hue_moving = effect.phase * HSV_HUE_MAX;
    float hue_mul = 0;
    if (effect.periods != 0)
      hue_mul = HSV_HUE_MAX/(((float)effect.pixels)/effect.periods);
    uint8_t *ptr = strip.Pixels();
    for (uint16_t i = 0; i < effect.pixels; i++) {
      uint16_t hue_temp = ((float)i)*hue_mul;
      uint16_t hue = (hue_temp + hue_moving) % HSV_HUE_MAX;
      //fast_hsv2rgb_32bit(hue, 255, value, ptr++, ptr++, ptr++);
      fast_hsv2rgb_32bit(hue, 255, 128, ptr++, ptr++, ptr++);
    }
    strip.Dirty();
    if (irrecv.isIdle()) strip.Show();
    //strip.Show();
  }
/*
  if (effect.mode == MODE_BINARY || effect.mode == MODE_OFF) {
    delay(4); //Arduino
  }

  if (effect.mode == MODE_VUMETER && power) {
    #define SENSORPIN A0
    int sensorValue = analogRead(SENSORPIN);
    strip.ClearTo(RgbColor(0));
    strip.ClearTo(effect.color, 0, sensorValue * effect.pixels / 1023);
    strip.Show();
    delay(4);
  }
*/  
  pollSerial();

  frame++;
  effect.phase += effect.speed/((float)(micros()-old_t));
  old_t = micros();
  effect.phase = fmod(effect.phase, 1.0f);
  if (effect.phase < 0.0f) effect.phase = 1.0f - effect.phase;

  //delay(4);

  decode_results results;        // Somewhere to store the results
  if (irrecv.decode(&results)) {  // Grab an IR code
    #ifdef DEBUG
    Serial.print("code: ");
    Serial.println(results.value, HEX);
    //dumpRaw(&results);            // Output the results in RAW format
    dumpCode(&results);           // Output the results as source code
    Serial.println("");           // Blank line between entries
    #endif
    irrecv.resume();              // Prepare for the next value
    processCommand(results.value);
  }

}

void processCommand(uint32_t cmd) {
  switch(cmd) {
    case POWER_BTN:
      power ^= 1;
      if (!power) clearStrip();
      debug("POWER_BTN");
      break;
    case RESET_BTN:
      effect.mode = MODE_RAINBOW;
      effect.speed = 6.0f;
      effect.periods = 1.0f;
      effect.brightness = MAX_BRIGHTNESS;
      power = 1;
      debug("RESET_BTN");
      break;
    case SPEED_DOWN_BTN:
      effect.speed -= 3.0f;
      debug("SPEED_DOWN_BTN");
      debug(effect.speed);
      break;
    case SPEED_UP_BTN:
      effect.speed += 3.0f;
      debug("SPEED_UP_BTN");
      debug(effect.speed);
      break;
    case SAVE_BTN:
      EEPROM.put(1024, effect);
      debug("SAVE_BTN");
      break;
    case WIDE_BTN:
      effect.periods -= 1.0f;
      debug("WIDE_BTN");
      break;
    case NARROW_BTN:
      effect.periods += 1.0f;
      debug("NARROW_BTN");
      break;
  }
  ensureEffectSanity();
  //printStatus();
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

void pollSerial() {
  if (!Serial.available()) return;
  if (Serial.peek() == 'b') {
    Serial.read(); //throw 'b' away
    effect.mode = MODE_BINARY;
    char *ptr = (char*)strip.Pixels();
    uint16_t cnt = Serial.readBytes(ptr, maxchars);
    strip.Dirty();
    strip.Show();
  } else {
    /*DynamicJsonBuffer jsonBuffer(255); //ArduinoJson 5
    JsonObject& root = jsonBuffer.parseObject(Serial);
    parseStripParameters(root);*/
  }

}

/*void parseStripParameters(JsonObject& root) {
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
    IFKEY("phase") {
      effect.phase = root["phase"];
      Serial.println("PHASE!");
    }
    IF("fading", "true") {
      effect.fading = 1;
      fading = 1;
    } else {
      effect.fading = 0;
      fading = 0;
    }
    IF("save", "true") {
      EEPROM.put(1024, effect);
      Serial.println("SAVE!");
    }
    IFKEY("load") {
      EEPROM.get(1024, effect);
    }
    ensureEffectSanity();
    printStatus();
}*/

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

void  dumpRaw (decode_results *results) {
  // Print Raw data
  Serial.print("Timing[");
  Serial.print(results->rawlen-1, DEC);
  Serial.println("]: ");

  for (int i = 1;  i < results->rawlen;  i++) {
    unsigned long  x = results->rawbuf[i] * USECPERTICK;
    if (!(i & 1)) {  // even
      Serial.print("-");
      if (x < 1000)  Serial.print(" ") ;
      if (x < 100)   Serial.print(" ") ;
      Serial.print(x, DEC);
    } else {  // odd
      Serial.print("     ");
      Serial.print("+");
      if (x < 1000)  Serial.print(" ") ;
      if (x < 100)   Serial.print(" ") ;
      Serial.print(x, DEC);
      if (i < results->rawlen-1) Serial.print(", "); //',' not needed for last one
    }
    if (!(i % 8))  Serial.println("");
  }
  Serial.println("");                    // Newline
}

void  dumpCode(decode_results *results) {
  // Start declaration
  Serial.print("unsigned int  ");          // variable type
  Serial.print("rawData[");                // array name
  Serial.print(results->rawlen - 1, DEC);  // array size
  Serial.print("] = {");                   // Start declaration

  // Dump data
  for (int i = 1;  i < results->rawlen;  i++) {
    Serial.print(results->rawbuf[i] * USECPERTICK, DEC);
    if ( i < results->rawlen-1 ) Serial.print(","); // ',' not needed on last one
    if (!(i & 1))  Serial.print(" ");
  }

  // End declaration
  Serial.print("};");  // 

  // Comment
  //Serial.print("  // ");
  //encoding(results);
  //Serial.print(" ");
  //ircode(results);

  // Newline
  Serial.println("");

  // Now dump "known" codes
  if (results->decode_type != UNKNOWN) {

    // Some protocols have an address
    if (results->decode_type == PANASONIC) {
      Serial.print("unsigned int  addr = 0x");
      Serial.print(results->address, HEX);
      Serial.println(";");
    }

    // All protocols have data
    Serial.print("unsigned int  data = 0x");
    Serial.print(results->value, HEX);
    Serial.println(";");
  }
}
