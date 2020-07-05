#include <EEPROM.h>
#include <FastLED.h>
#include "BluetoothSerial.h"
#include <ArduinoJson.h>

#define LED_PIN     14
//#define NUM_LEDS    50
#define NUM_LEDS    300
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];
#define UPDATES_PER_SECOND 100
//#define UPDATES_PER_SECOND 20

CRGBPalette16 currentPalette;
TBlendType    currentBlending;
extern CRGBPalette16 myRedWhiteBluePalette;
extern const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM;

BluetoothSerial SerialBT;
StaticJsonDocument<1024> doc;
String inputStr;

#define LED_BUILTIN 2

#define HOWMANYMODES 4
#define MODE_STATIC 1
#define MODE_RAINBOW 2
#define MODE_PALETTE 3
#define MODE_RAIN 4

struct {
  //uint16_t pixels;
  uint8_t mode;
  bool onOff;
  uint8_t brightness; //0.0...255.0 (CSHV minimum 23 without any hue to go black)
  float speed; //-3.0...3.0
  float hue; //0.0...255.0
  float saturation; //0.0...255.0
  uint32_t lastMillis;
  float counter;
  float freq; //0.0-10.0
  //float periods;
  //uint8_t fading;
  //RgbColor color;
  //float phase;
  //uint16_t checksum;
} effect;

void setup() {
  Serial.begin(115200);
  SerialBT.begin("ESP32test"); //Bluetooth device name
  Serial.println("The device started, now you can pair it with bluetooth!");
  pinMode(LED_BUILTIN, OUTPUT);

  EEPROM.begin(sizeof(effect));
  EEPROM.get(0, effect);
  //EEPROM.end();

  if (effect.mode == 0 || effect.mode > HOWMANYMODES) { // crude "sanity check"
    effect.mode = MODE_STATIC;
    effect.onOff = true;
    effect.brightness = 32;
    effect.speed = 1;
    effect.hue = 0;
    effect.saturation = 255;
    effect.freq = 1;
  }

  Serial.println(effect.mode);
  Serial.println(effect.onOff);
  Serial.println(effect.brightness);
  Serial.println(effect.speed);
  Serial.println(effect.hue);
  Serial.println(effect.saturation);
  Serial.println(effect.freq);
  Serial.print("sizeof(effect): "); Serial.println(sizeof(effect));

  //delay(3000); // power-up safety delay
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setDither(0);
  //FastLED.setBrightness(effect.brightness);
  currentPalette = RainbowColors_p;
  currentBlending = LINEARBLEND;

  updateEffect();

  /*Serial.println(micros());
  for (int c = 0; c < 100; c++) {
    //uint8_t startHue = (((float)millis()) / 1000.0 * 255.0 * effect.speed);
    //fill_rainbow(leds, NUM_LEDS, startHue, (uint8_t)(255.0/50.0));
    //for (int i = 0; i < NUM_LEDS; i++) leds[i].setHue((uint8_t)((float)i * NUM_LEDS / 255.0) + c);
    uint8_t startHue = c << 8;
    for (uint16_t i = 0; i < NUM_LEDS; i++) leds[i].setHue((i * NUM_LEDS + startHue) >> 8); // < 10ms
  }
  Serial.println(micros());*/
  
}

void loop() {
  if (Serial.available()) {
    SerialBT.write(Serial.read());
  }
  if (SerialBT.hasClient()) {
    digitalWrite(LED_BUILTIN, HIGH);
  } else {
    digitalWrite(LED_BUILTIN, LOW);
  }
  if (SerialBT.available()) {
    inputStr = SerialBT.readStringUntil('\n');
    Serial.println("Message received from BT");
    Serial.println(inputStr);
    
    const char *s = inputStr.c_str();
    char c = s[0];
    switch (c) {
      case 'a':
        digitalWrite(LED_BUILTIN, HIGH);
        break;
      case 'b':
        digitalWrite(LED_BUILTIN, LOW);
        break;
      case '*':
        doc["mode"] = effect.mode;
        if (effect.onOff) doc["onOff"] = "true"; else doc["onOff"] = "false";
        doc["brightness"] = effect.brightness;
        doc["speed"] = effect.speed;
        doc["hue"] = effect.hue;
        doc["saturation"] = effect.saturation;
        doc["freq"] = effect.freq;
        serializeJson(doc, SerialBT); SerialBT.write('\n');
        break;
      default:
        parseEffect();
        updateEffect();
    }
  }

  if (effect.mode > 1 && effect.onOff == true) updateEffect();
  //FastLED.delay(1000 / UPDATES_PER_SECOND);


}

// https://arduinojson.org/v5/api/jsonobject/containskey/
#define IFKEY(a) if (doc.containsKey(a))
#define IF(a,b) IFKEY(a) if (!strcmp(doc[a],b)) // DON'T USE! Guru Meditation Error: Core  1 panic'ed (LoadProhibited). Exception was unhandled.

void parseEffect() {
  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, inputStr);
  // Test if parsing succeeds.
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return;
  }
  IFKEY("brightness") effect.brightness = doc["brightness"];
  //FastLED.setBrightness(effect.brightness);
  IFKEY("onOff") effect.onOff = doc["onOff"];
  if (!effect.onOff) {
    fill_solid(leds, NUM_LEDS, CRGB(0, 0, 0));
    FastLED.show();
  }
  IFKEY("mode") effect.mode = doc["mode"];
  IFKEY("speed") effect.speed = doc["speed"];
  IFKEY("hue") effect.hue = doc["hue"];
  IFKEY("saturation") effect.saturation = doc["saturation"];
  IFKEY("freq") effect.freq = doc["freq"];
  /*IF("save", "true") {
    Serial.println("Saving");
    //EEPROM.write(0, &effect);
    //EEPROM.begin(512);
    //Effect foo;
    //EEPROM.put(0, effect);
    //EEPROM.put(0, foo);
    //EEPROM.commit();
    //EEPROM.end();
    //EEPROM.commit();
  }*/
  IFKEY("save") {
    Serial.println("Saving");
    EEPROM.put(0, effect);
    EEPROM.commit();
  }
}

void updateEffect() {
  //if (effect.onOff) {
    float now = millis();
    float diff = now - effect.lastMillis; effect.lastMillis = now;
    effect.counter = effect.counter + diff * effect.speed;
    uint32_t startIndex = effect.counter * NUM_LEDS / 1000.0 * 256.0;
    uint16_t startHue = effect.counter * NUM_LEDS / 1000.0;
    uint8_t hue = effect.hue;
    uint8_t brightness = effect.brightness;
    uint16_t freq = effect.freq * 256.0;
    uint8_t saturation = effect.saturation;
    switch (effect.mode) {
      case MODE_STATIC:
        fill_solid(leds, NUM_LEDS, CHSV(hue, saturation, brightness));
        //fadeLightBy(leds, NUM_LEDS, 255-brightness);
        //fadeToBlackBy(leds, NUM_LEDS, 255-brightness);
        //fill_rainbow(leds, NUM_LEDS, 0, (uint8_t)(255.0/50.0));
        FastLED.show();
        FastLED.delay(1000 / UPDATES_PER_SECOND);
        break;
      case MODE_RAINBOW:
        for (uint16_t i = 0; i < NUM_LEDS; i++) {
          //leds[i].setHue((i * NUM_LEDS + startHue) >> 8);
          //leds[i].setHSV((i * NUM_LEDS + startHue) >> 8, 255, brightness);
          leds[i].setHSV(((i * freq) >> 8) + startHue, 255, brightness);
        }
        FastLED.show();
        FastLED.delay(1000 / UPDATES_PER_SECOND);
        break;
      case MODE_PALETTE:
        currentPalette = CloudColors_p;           
        currentBlending = LINEARBLEND;
        for(int i = 0; i < NUM_LEDS; i++) {
          leds[i] = ColorFromPalette(currentPalette, startIndex >> 8, brightness, currentBlending);
          startIndex += freq>>2;
        }
        //FillLEDsFromPaletteColors(startIndex);
        //fadeToBlackBy(leds, NUM_LEDS, 255-brightness);
        FastLED.show();
        FastLED.delay(1000 / UPDATES_PER_SECOND);
        break;
      case MODE_RAIN: //bright=110,freq=7.6,hue=183,sat=128,20fps
        //fadeToBlackBy(leds, NUM_LEDS, 20); // 5/90/255,255,255 oli jo aika hyvä. ja 10/90/150,150,255
        //if (random(100) > 80) leds[random(0,NUM_LEDS-1)] = CRGB(150,150,200); //ja 20/80/150,150,200
        //fadeToBlackBy(leds, NUM_LEDS, (effect.speed+3.0)*4.0);
        blur1d(leds, NUM_LEDS, (effect.speed+3.0)*42.0);
        if (random(100) > (100.0-(effect.freq*10.0))) {
          int16_t x = random(0, NUM_LEDS-2);
          leds[x] = leds[x+1] = CHSV(hue, saturation, brightness);
        }
        FastLED.show();
        //FastLED.delay(1000 / UPDATES_PER_SECOND);
        FastLED.delay(1000 / 20);
        break;
    }
  /*} else { //off:
    //clear(leds, NUM_LEDS);
    fill_solid(leds, NUM_LEDS, CRGB(0, 0, 0));
    FastLED.show();
  }*/
}

// ------------------------------------------------
// FastLED
// ------------------------------------------------
void FillLEDsFromPaletteColors(uint8_t colorIndex)
{
    uint8_t brightness = 255;
    
    for( int i = 0; i < NUM_LEDS; i++) {
        leds[i] = ColorFromPalette(currentPalette, colorIndex, brightness, currentBlending);
        colorIndex += 3;
    }
}


// There are several different palettes of colors demonstrated here.
//
// FastLED provides several 'preset' palettes: RainbowColors_p, RainbowStripeColors_p,
// OceanColors_p, CloudColors_p, LavaColors_p, ForestColors_p, and PartyColors_p.
//
// Additionally, you can manually define your own color palettes, or you can write
// code that creates color palettes on the fly.  All are shown here.

void ChangePalettePeriodically()
{
    uint8_t secondHand = (millis() / 1000) % 60;
    static uint8_t lastSecond = 99;
    
    if( lastSecond != secondHand) {
        lastSecond = secondHand;
        if( secondHand ==  0)  { currentPalette = RainbowColors_p;         currentBlending = LINEARBLEND; }
        if( secondHand == 10)  { currentPalette = RainbowStripeColors_p;   currentBlending = NOBLEND;  }
        if( secondHand == 15)  { currentPalette = RainbowStripeColors_p;   currentBlending = LINEARBLEND; }
        if( secondHand == 20)  { SetupPurpleAndGreenPalette();             currentBlending = LINEARBLEND; }
        if( secondHand == 25)  { SetupTotallyRandomPalette();              currentBlending = LINEARBLEND; }
        if( secondHand == 30)  { SetupBlackAndWhiteStripedPalette();       currentBlending = NOBLEND; }
        if( secondHand == 35)  { SetupBlackAndWhiteStripedPalette();       currentBlending = LINEARBLEND; }
        if( secondHand == 40)  { currentPalette = CloudColors_p;           currentBlending = LINEARBLEND; }
        if( secondHand == 45)  { currentPalette = PartyColors_p;           currentBlending = LINEARBLEND; }
        if( secondHand == 50)  { currentPalette = myRedWhiteBluePalette_p; currentBlending = NOBLEND;  }
        if( secondHand == 55)  { currentPalette = myRedWhiteBluePalette_p; currentBlending = LINEARBLEND; }
    }
}

// This function fills the palette with totally random colors.
void SetupTotallyRandomPalette()
{
    for( int i = 0; i < 16; i++) {
        currentPalette[i] = CHSV( random8(), 255, random8());
    }
}

// This function sets up a palette of black and white stripes,
// using code.  Since the palette is effectively an array of
// sixteen CRGB colors, the various fill_* functions can be used
// to set them up.
void SetupBlackAndWhiteStripedPalette()
{
    // 'black out' all 16 palette entries...
    fill_solid( currentPalette, 16, CRGB::Black);
    // and set every fourth one to white.
    currentPalette[0] = CRGB::White;
    currentPalette[4] = CRGB::White;
    currentPalette[8] = CRGB::White;
    currentPalette[12] = CRGB::White;
    
}

// This function sets up a palette of purple and green stripes.
void SetupPurpleAndGreenPalette()
{
    CRGB purple = CHSV( HUE_PURPLE, 255, 255);
    CRGB green  = CHSV( HUE_GREEN, 255, 255);
    CRGB black  = CRGB::Black;
    
    currentPalette = CRGBPalette16(
                                   green,  green,  black,  black,
                                   purple, purple, black,  black,
                                   green,  green,  black,  black,
                                   purple, purple, black,  black );
}


// This example shows how to set up a static color palette
// which is stored in PROGMEM (flash), which is almost always more
// plentiful than RAM.  A static PROGMEM palette like this
// takes up 64 bytes of flash.
const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM =
{
    CRGB::Red,
    CRGB::Gray, // 'white' is too bright compared to red and blue
    CRGB::Blue,
    CRGB::Black,
    
    CRGB::Red,
    CRGB::Gray,
    CRGB::Blue,
    CRGB::Black,
    
    CRGB::Red,
    CRGB::Red,
    CRGB::Gray,
    CRGB::Gray,
    CRGB::Blue,
    CRGB::Blue,
    CRGB::Black,
    CRGB::Black
};



// Additional notes on FastLED compact palettes:
//
// Normally, in computer graphics, the palette (or "color lookup table")
// has 256 entries, each containing a specific 24-bit RGB color.  You can then
// index into the color palette using a simple 8-bit (one byte) value.
// A 256-entry color palette takes up 768 bytes of RAM, which on Arduino
// is quite possibly "too many" bytes.
//
// FastLED does offer traditional 256-element palettes, for setups that
// can afford the 768-byte cost in RAM.
//
// However, FastLED also offers a compact alternative.  FastLED offers
// palettes that store 16 distinct entries, but can be accessed AS IF
// they actually have 256 entries; this is accomplished by interpolating
// between the 16 explicit entries to create fifteen intermediate palette
// entries between each pair.
//
// So for example, if you set the first two explicit entries of a compact 
// palette to Green (0,255,0) and Blue (0,0,255), and then retrieved 
// the first sixteen entries from the virtual palette (of 256), you'd get
// Green, followed by a smooth gradient from green-to-blue, and then Blue.
