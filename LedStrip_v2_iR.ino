// https://github.com/Makuna/NeoPixelBus
// https://github.com/Makuna/NeoPixelBus/wiki/NeoPixelBus-object-API
// https://github.com/Makuna/NeoPixelBus/wiki/HslColor-object-API

#include <NeoPixelBus.h>
//#include <ArduinoJson.h>
#include <EEPROM.h>
#include <IRremote.h>

#define recvPin 3
#define GND 4
#define VCC 5
IRrecv irrecv(recvPin);

const uint8_t PixelPin = 2;

#define MAX_PIXELS 100
#define MAX_BRIGHTNESS 128

//#define DEBUG
#undef DEBUG

#ifdef DEBUG
#define DEBUG_MSG(x) Serial.println(x)
#else
#define DEBUG_MSG(x)
#endif

#define POWER_BTN 0xFFB04F // power (red)
#define RESET_BTN 0xFFF807 // w/ww (black)
#define SPEED_DOWN_BTN 0xFF48B7 // <<<
#define SPEED_UP_BTN 0xFF6897 // >>>
#define SAVE_BTN 0xFF9867 // IC set
#define LOAD_BTN 0xFFA857 // Auto
#define WIDE_BTN 0xFFD827 // < | > (green)
#define NARROW_BTN 0xFF8877 // > | < (blue)
#define BRIGHTNESS_UP_BTN 0xFF906F // up
#define BRIGHTNESS_DOWN_BTN 0xFFB847 // down
#define FLASH_BTN 0xFF00FF // Flash (turquoise)
#define FILL_BTN 0xFFB24D // <-/-> (yellow)
#define HUE_LEFT_BTN 0xFF28D7 // X*-.
#define HUE_RIGHT_BTN 0xFFF00F // .-*X
#define TRAVELLER_BTN 0xFF30CF // Meteor
#define GLITTER_BTN 0xFF58A7 // Jump
#define ZERO_SATURATION_BTN 0xFF38C7 // C16
#define LOW_SATURATION_BTN 0xFF50AF // C7
#define HIGH_SATURATION_BTN 0xFF02FD // C3
#define FULL_SATURATION_BTN 0xFFE817 // CS
#define VARIATION_LEFT_BTN 0xFF32CD // <*
#define VARIATION_RIGHT_BTN 0xFF20DF // *>

uint16_t maxchars;
uint32_t frame;
uint8_t fading = 1;
uint8_t power = 1;
uint32_t old_t = 0;
uint32_t frame_duration = 0;

//#define MODE_OFF 0
#define MODE_RAINBOW 0
#define MODE_FILL 1
#define MODE_BINARY 2 // get values from serial port as raw binary. prefix raw data with byte 'b'.
#define MODE_BLINK 3
#define MODE_TRAVELLER 4
#define MODE_GLITTER 5
#define BIGGEST_MODE_NUMBER 5

#define VARIATION_SINGLE 1
#define VARIATION_RAINBOW 2
#define VARIATION_PALETTE 3

#define HSV_HUE_SEXTANT    256
#define HSV_HUE_STEPS   (6 * HSV_HUE_SEXTANT)

#define HSV_HUE_MIN   0
#define HSV_HUE_MAX   (HSV_HUE_STEPS - 1)
#define HSV_SAT_MIN   0
#define HSV_SAT_MAX   255
#define HSV_VAL_MIN   0
#define HSV_VAL_MAX   255

struct Effect {
  uint16_t pixels;
  uint8_t mode;
  int16_t brightness;
  float speed;
  float periods;
  uint8_t fading;
  RgbColor color;
  float hue;
  float saturation;
  float phase;
  uint8_t variation;
  uint16_t checksum;
} effect;

uint8_t buf1[MAX_PIXELS];
uint32_t buf2[MAX_PIXELS];

void ensureEffectSanity() {
  if (effect.pixels > MAX_PIXELS) effect.pixels = MAX_PIXELS;
  if (effect.pixels < 4) effect.pixels = 4;
  if (effect.mode > BIGGEST_MODE_NUMBER) effect.mode = 1;
  if (effect.brightness <= 0) effect.brightness = 1;
  if (effect.brightness > MAX_BRIGHTNESS) effect.brightness = MAX_BRIGHTNESS;
  if (effect.periods < 0) effect.periods = 0;
  effect.hue = fmod(effect.hue, 1.0f); if (effect.hue < 0.0f) effect.hue = 1.0f - effect.hue;
  if (effect.saturation < 0) effect.saturation = 0; if (effect.saturation > 1.0f) effect.saturation = 1.0f;
  if (effect.fading != 1) effect.fading = 0;
  maxchars = effect.pixels*3;
}

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

  strip.Begin();

  Serial.println();
  Serial.println("Running...");
  DEBUG_MSG("DEBUG!");

  EEPROM.get(1024, effect);
  ensureEffectSanity();
  
  clearStrip();
  
  frame = 0;
  fading = effect.fading;

  printStatus();
  
  irrecv.enableIRIn();  // Start the receiver

  /*for(uint16_t i = 0; i < MAX_PIXELS; i++) {
    float c = ((float)i) / ((float)MAX_PIXELS);
    HsbColor color = HsbColor(c, 1.0f, 1.0f);
    strip.SetPixelColor(i, color);
  }
  strip.Show();*/
}


void loop() {
  //uint16_t value = handleFading();

  if (effect.mode == MODE_RAINBOW && power) {
    uint16_t hue_moving = effect.phase * 6.0f * HSV_HUE_MAX;
    float hue_mul = 0;
    if (effect.periods != 0)
      hue_mul = HSV_HUE_MAX/(((float)effect.pixels)/effect.periods);
    uint8_t *ptr = strip.Pixels();
    for (uint16_t i = 0; i < effect.pixels; i++) {
      uint16_t hue_temp = ((float)i)*hue_mul;
      uint16_t hue = (hue_temp + hue_moving) % HSV_HUE_MAX;
      fast_hsv2rgb_32bit(hue, effect.saturation*255, effect.brightness, ptr++, ptr++, ptr++);
    }
    strip.Dirty();
    if (irrecv.isIdle()) strip.Show();
  }

  if (effect.mode == MODE_FILL && power) {
    HslColor color = HslColor(effect.hue, effect.saturation, effect.brightness/255.0f);
    strip.ClearTo(color);
    if (irrecv.isIdle()) strip.Show();
    delay(100);
  }
  
  if (effect.mode == MODE_BLINK && power) {
    uint8_t *ptr = strip.Pixels();
    uint8_t value = frame_duration / 5000;
    for (uint16_t i = 0; i < effect.pixels*3; i++) {
      if ((ptr[i] - value) > 0) ptr[i] -= value; else ptr[i] = 0;
    }
    strip.Dirty();
    if (effect.periods < 1) effect.periods = 1;
    uint8_t new_blink = frame%(50/(uint32_t)effect.periods);
    if (new_blink == 0) {
      HsbColor color;
      if (effect.variation == VARIATION_SINGLE) 
        color = HsbColor(effect.hue, effect.saturation, effect.brightness/255.0f);
      else
        color = HsbColor((float)(frame%1000)/1000, effect.saturation, effect.brightness/255.0f);
      strip.SetPixelColor(random(effect.pixels), color);
    }
    if (irrecv.isIdle()) strip.Show();
  }

  if (effect.mode == MODE_TRAVELLER && power) {
    uint8_t *ptr = strip.Pixels();
    uint8_t value = frame_duration / 2000;
    for (uint16_t i = 0; i < effect.pixels*3; i++) {
      if ((ptr[i] - value) > 0) ptr[i] -= value; else ptr[i] = 0;
    }
    strip.Dirty();
    uint16_t p = (uint32_t)(frame*effect.speed/20) % effect.pixels;
    HslColor color;
    if (effect.variation == VARIATION_SINGLE)
      color = HslColor(effect.hue, effect.saturation, effect.brightness/255.0f);
    else
      color = HslColor((float)(frame%1000)/1000, effect.saturation, effect.brightness/255.0f);
    strip.SetPixelColor(p, color);
    if (irrecv.isIdle()) strip.Show();
  }

  if (effect.mode == MODE_GLITTER && power) {
    /*for (int i = 0; i < effect.pixels; i++) {
      RgbColor color = strip.GetPixelColor(i);
      color.Darken(1);
      strip.SetPixelColor(i, color);
    }*/
    uint8_t *ptr = strip.Pixels();
    for (uint16_t i = 0; i < effect.pixels; i++) {
      buf1[i] = strip.GetPixelColor(i).CalculateBrightness();
    }
    for (uint16_t i = 0; i < effect.pixels; i++) {
      if (buf2[i] != 0) {
        int value = (micros() - buf2[i]) / 5000;
        if (value > effect.brightness) value = effect.brightness; if (value < 0) value = 0;
        HsbColor color;
        if (effect.variation == VARIATION_SINGLE) 
          color = HsbColor(effect.hue, effect.saturation, value/255.0f);
        else
          color = HsbColor((float)(frame%1000)/1000, effect.saturation, value/255.0f);
        strip.SetPixelColor(i, color);
        if ((micros() - buf2[i]) > 1000000) buf2[i] = 0;
      } else {
        RgbColor color = strip.GetPixelColor(i);
        color.Darken(1);
        strip.SetPixelColor(i, color);
      }
    }
    if (effect.periods < 1) effect.periods = 1;
    uint8_t new_blink = frame%(50/(uint32_t)effect.periods);
    if (new_blink == 0) {
      int p;
      do {
        p = random(effect.pixels);
      } while (strip.GetPixelColor(p).CalculateBrightness() != 0);
      buf2[p] = micros();
    }
    strip.Dirty();
    if (irrecv.isIdle()) strip.Show();
  }
  
  delay(10);

  pollSerial();

  frame++;
  frame_duration = micros()-old_t;
  effect.phase += effect.speed/((float)frame_duration);
  old_t = micros();
  effect.phase = fmod(effect.phase, 1.0f);
  if (effect.phase < 0.0f) effect.phase = 1.0f - effect.phase;

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
      DEBUG_MSG("POWER_BTN");
      break;
    case RESET_BTN:
      effect.mode = MODE_RAINBOW;
      effect.pixels = MAX_PIXELS;
      effect.speed = 1.0f;
      effect.periods = 1.0f;
      effect.phase = 0.0f;
      effect.brightness = MAX_BRIGHTNESS;
      effect.saturation = 1.0f;
      effect.variation = VARIATION_SINGLE;
      power = 1;
      DEBUG_MSG("RESET_BTN");
      break;
    case SPEED_DOWN_BTN:
      effect.speed -= 1.0f;
      DEBUG_MSG("SPEED_DOWN_BTN");
      DEBUG_MSG(effect.speed);
      break;
    case SPEED_UP_BTN:
      effect.speed += 1.0f;
      DEBUG_MSG("SPEED_UP_BTN");
      DEBUG_MSG(effect.speed);
      break;
    case SAVE_BTN:
      EEPROM.put(1024, effect);
      DEBUG_MSG("SAVE_BTN");
      break;
    case LOAD_BTN:
      EEPROM.get(1024, effect);
      clearStrip();
      DEBUG_MSG("LOAD_BTN");
      break;
    case WIDE_BTN:
      effect.periods -= 1.0f;
      DEBUG_MSG("WIDE_BTN");
      break;
    case NARROW_BTN:
      effect.periods += 1.0f;
      DEBUG_MSG("NARROW_BTN");
      break;
    case BRIGHTNESS_UP_BTN:
      effect.brightness += 10;
      DEBUG_MSG("BRIGHTNESS_UP_BTN");
      break;
    case BRIGHTNESS_DOWN_BTN:
      effect.brightness -= 10;
      DEBUG_MSG("BRIGHTNESS_DOWN_BTN");
      break;
    case FLASH_BTN:
      effect.mode = MODE_BLINK;
      DEBUG_MSG("FLASH_BTN");
      break;
    case FILL_BTN:
      effect.mode = MODE_FILL;
      DEBUG_MSG("FILL_BTN");
      break;
    case HUE_LEFT_BTN:
      effect.hue -= 0.05;
      DEBUG_MSG("HUE_LEFT_BTN");
      break;
    case HUE_RIGHT_BTN:
      effect.hue += 0.05;
      DEBUG_MSG("HUE_RIGHT_BTN");
      break;
    case TRAVELLER_BTN:
      effect.mode = MODE_TRAVELLER;
      DEBUG_MSG("TRAVELLER_BTN");
      break;
    case GLITTER_BTN:
      effect.mode = MODE_GLITTER;
      clearStrip();
      DEBUG_MSG("GLITTER_BTN");
      break;
    case ZERO_SATURATION_BTN:
      effect.saturation = 0.0f;
      DEBUG_MSG("ZERO_SATURATION_BTN");
      break;
    case LOW_SATURATION_BTN:
      effect.saturation = 0.4f;
      DEBUG_MSG("LOW_SATURATION_BTN");
      break;
    case HIGH_SATURATION_BTN:
      effect.saturation = 0.8f;
      DEBUG_MSG("HIGH_SATURATION_BTN");
      break;
    case FULL_SATURATION_BTN:
      effect.saturation = 1.0f;
      DEBUG_MSG("FULL_SATURATION_BTN");
      break;
    case VARIATION_LEFT_BTN:
      effect.variation = VARIATION_SINGLE;
      DEBUG_MSG("VARIATION_LEFT_BTN");
      break;
    case VARIATION_RIGHT_BTN:
      effect.variation = VARIATION_RAINBOW;
      DEBUG_MSG("VARIATION_RIGHT_BTN");
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
  for (int i = 0; i < MAX_PIXELS; i++) { buf1[i] = 0; buf2[i] = 0; }
  strip.ClearTo(RgbColor(0));
  if (irrecv.isIdle()) strip.Show();
}

/*void clearStrip(HslColor color) {
  strip.ClearTo(color);
  if (irrecv.isIdle()) strip.Show();
}*/

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


/* http://cpp.sh/
 * // Example program
#include <iostream>
#include <string>

using namespace std;
uint32_t frame;

int main()
{
    cout<<"hello world\n";
    uint8_t value = 0;
    frame = 12345678;
    value = frame;
    //value = 100;
    
    cout << hex<<static_cast<int>(frame) <<"\n";
    cout << hex<<static_cast<int>(value);
    
    return 0;
}

hello world
bc614e
4e 
*/
