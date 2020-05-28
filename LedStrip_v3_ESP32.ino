#include <FastLED.h>
#include "BluetoothSerial.h"
#include <ArduinoJson.h>

#define LED_PIN     14
//#define NUM_LEDS    50
#define NUM_LEDS    300
//#define BRIGHTNESS  64
#define BRIGHTNESS  10
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];
#define UPDATES_PER_SECOND 100

CRGBPalette16 currentPalette;
TBlendType    currentBlending;
extern CRGBPalette16 myRedWhiteBluePalette;
extern const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM;

BluetoothSerial SerialBT;
StaticJsonDocument<1024> doc;
String inputStr;

#define LED_BUILTIN 2

#define MODE_STATIC 1
#define MODE_RAINBOW 2

struct Effect {
  //uint16_t pixels;
  uint8_t mode;
  bool onOff;
  uint8_t brightness;
  float speed;
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

  effect.mode = MODE_STATIC;
  effect.onOff = true;
  effect.brightness = 10;
  effect.speed = 1;

  //delay(3000); // power-up safety delay
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);
  currentPalette = RainbowColors_p;
  currentBlending = LINEARBLEND;

  updateEffect();

  Serial.println(micros());
  for (int c = 0; c < 100; c++) {
    //uint8_t startHue = (((float)millis()) / 1000.0 * 255.0 * effect.speed);
    //fill_rainbow(leds, NUM_LEDS, startHue, (uint8_t)(255.0/50.0));
    //for (int i = 0; i < NUM_LEDS; i++) leds[i].setHue((uint8_t)((float)i * NUM_LEDS / 255.0) + c);
    uint8_t startHue = c << 8;
    for (uint16_t i = 0; i < NUM_LEDS; i++) leds[i].setHue((i * NUM_LEDS + startHue) >> 8);
  }
  Serial.println(micros());
  
}

void loop() {
  if (Serial.available()) {
    SerialBT.write(Serial.read());
  }
  if (SerialBT.available()) {
    inputStr = SerialBT.readStringUntil('\n');
    Serial.println(inputStr);
    
    //char s[100];
    //S.toCharArray(s, 99);
    //for (int i=0; i < sizeof(S)+5; i++) { Serial.println(s[i], DEC); }
    //char c = SerialBT.read();
    const char *s = inputStr.c_str();
    char c = s[0];
    //Serial.write(c);
    switch (c) {
      case 'a':
        digitalWrite(LED_BUILTIN, HIGH);
        break;
      case 'b':
        digitalWrite(LED_BUILTIN, LOW);
        break;
      case '*':
        //if (digitalRead(LED_BUILTIN)) doc["led"] = "true"; else doc["led"] = "false";
        if (effect.onOff) doc["onOff"] = "true"; else doc["onOff"] = "false";
        doc["brightness"] = effect.brightness;
        doc["speed"] = effect.speed;
        serializeJson(doc, SerialBT); SerialBT.write('\n');
        break;
      default:
        parseEffect();
        updateEffect();
    }
  }

  if (effect.mode > 1 && effect.onOff == true) updateEffect();
  //delay(20);


}

// https://arduinojson.org/v5/api/jsonobject/containskey/
#define IFKEY(a) if (doc.containsKey(a))
#define IF(a,b) IFKEY(a) if (!strcmp(doc[a],b))

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
  FastLED.setBrightness(effect.brightness);
  IFKEY("onOff") effect.onOff = doc["onOff"];
  IFKEY("mode") effect.mode = doc["mode"];
  IFKEY("speed") effect.speed = doc["speed"];
}

void updateEffect() {
  if (effect.onOff) {
    digitalWrite(LED_BUILTIN, HIGH);
    switch (effect.mode) {
      case MODE_STATIC:
        //ledcAnalogWrite(LEDC_CHANNEL_0, effect.brightness);
        /*currentPalette = CloudColors_p; currentBlending = LINEARBLEND;
        FillLEDsFromPaletteColors(0);
        FastLED.show();*/
        /*for (int i = 0; i < NUM_LEDS; i++) {
          leds[i] = CRGB(64, 0, 0);
        }
        //FastLED.showColor(CRGB(64, 0, 0));
        FastLED.setBrightness(effect.brightness);
        FastLED.show();*/
        //static uint8_t hue = 0;
        //FastLED.showColor(CHSV(hue++, 255, 255));
        //FastLED.showColor(CRGB::Green);
        fill_solid(leds, NUM_LEDS, CRGB(0,0,40));
        //fill_rainbow(leds, NUM_LEDS, 0, (uint8_t)(255.0/50.0));
        FastLED.show();
        FastLED.delay(1000 / UPDATES_PER_SECOND);
        break;
      case MODE_RAINBOW:
        //ChangePalettePeriodically();
        currentPalette = RainbowColors_p; currentBlending = LINEARBLEND;
        //static uint8_t startIndex = 0;
        //startIndex = startIndex + 1; // motion speed
        //uint16_t startIndex = ((millis() >> 4) * effect.speed);
        //uint16_t startIndex = (((float)millis()) / 1000.0 * 300.0 * effect.speed);
        //uint16_t startHue = (((float)millis()) / 1000.0 * 255.0 * effect.speed) << 8;
        uint16_t startHue = (millis() >> 2) * (uint16_t)(effect.speed*256);
        //uint8_t startHue = millis() >> 7;
        //uint16_t startIndex = (((float)millis()) / 1000.0 * NUM_LEDS * effect.speed);
        //FillLEDsFromPaletteColors(startIndex);
        //FastLED.setBrightness(effect.brightness);
        //fill_rainbow(leds, NUM_LEDS, startHue, (uint8_t)(255.0/NUM_LEDS));
        //fill_rainbow(leds, NUM_LEDS, startHue, 1);
        //FastLED.clear(leds);
        for (int i = 0; i < NUM_LEDS; i++) {
        //for (int i = 0; i < 150; i++) {
          //uint8_t hue = startHue + i; //((float)i * NUM_LEDS / 255.0);
          //leds[i] = CHSV(i, 255, 64);
          //leds[i] = CRGB(255, 0, i);
          //leds[i].setHue(i);
          //leds[i].setHue((uint8_t)((float)i * NUM_LEDS / 255.0) + startHue);
          leds[i].setHue((i * NUM_LEDS + startHue) >> 8);
        }
        FastLED.show();
        //delay(100);
        FastLED.delay(1000 / UPDATES_PER_SECOND);
        //updateRainbow();
        //uint8_t a = ((millis() >> 4) * effect.speed);
        //ledcAnalogWrite(LEDC_CHANNEL_0, a);
        break;
    }
  } else {
    digitalWrite(LED_BUILTIN, LOW);
  }
}

// ------------------------------------------------
// FastLED
// ------------------------------------------------
void FillLEDsFromPaletteColors( uint8_t colorIndex)
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
