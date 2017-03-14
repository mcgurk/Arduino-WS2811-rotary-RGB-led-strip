#include <EEPROM.h>

#include <Encoder.h>

#include <NeoPixelBus.h>

const uint16_t PixelCount = 60; // this example assumes 4 pixels, making it smaller will cause a failure
const uint8_t PixelPin = 2;  // make sure to set this to the correct pin, ignored for Esp8266

// three element pixels, in different order and speeds
NeoPixelBus<NeoRgbFeature, Neo800KbpsMethod> strip(PixelCount, PixelPin);

// Change these two numbers to the pins connected to your encoder.
//   Best Performance: both pins have interrupt capability
//   Good Performance: only the first pin has interrupt capability
//   Low Performance:  neither pin has interrupt capability
Encoder myEnc(12, 11);
//   avoid using pins with LEDs attached

// https://en.wikipedia.org/wiki/Additive_color
// http://www.vendian.org/mncharity/dir3/blackbody/UnstableURLs/bbr_color.html (10deg)
#define TEMPERATURES_COUNT 27
uint32_t temperatures_rgb[] = {
  0xff0000, //RED
  0x00ff00, //GREEN
  0x0000ff, //BLUE
  0xffff00, //YELLOW
  0x00ffff, //CYAN
  0xff00ff, //PURPLE
  0xffffff, //WHITE
  0xff3800, //1000K
  0xff5d00, //1300K
  0xff6d00, //1500K
  0xff7e00, //1800K
  0xff8912, //2000K
  0xff9836, //2300K
  0xffa148, //2500K
  0xffad5e, //2800K
  0xffb46b, //3000K
  0xffc987, //3500K
  0xffd1a3, //4000K
  0xffdbba, //4500K
  0xffe4ce, //5000K
  0xffece0, //5500K
  0xfff3ef, //6000K
  0xfff9fd, //6500K
  0xf5f3ff, //7000K
  0xe3e9ff, //8000K
  0xd6e1ff, //9000K
  0xccdbff  //10000K
};

uint8_t mode = 0;
long brightness;
long temperature;
long speed = 10;
long oldPosition  = -999;

void setup() {
  brightness = EEPROM.read(0);
  temperature = EEPROM.read(1);
  Serial.begin(115200);
  Serial.println(brightness);
  Serial.println(temperature);
  Serial.println("Basic Encoder Test:");
  pinMode(8, OUTPUT);
  digitalWrite(8, LOW);
  pinMode(9, OUTPUT);
  digitalWrite(9, HIGH);
  pinMode(10, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  // this resets all the neopixels to an off state
  strip.Begin();
  strip.Show();

  myEnc.write(brightness*4);
}

uint32_t frame = 0;
uint8_t colorcycle = 0;
#define MAX_BRIGHTNESS 40
#define MAX_SPEED 200
uint32_t last_millis;

void loop() {
  
  long newPosition = myEnc.read();
  if ( ((newPosition & B11) == 0) && (newPosition != oldPosition) ) {
    Serial.println(newPosition);
    long value = newPosition >> 2;
    oldPosition = newPosition;
    
    if (mode == 0) {
      if (value < 0) { value = 0; myEnc.write(0*4); }
      if (value > MAX_BRIGHTNESS) { value = MAX_BRIGHTNESS; myEnc.write(MAX_BRIGHTNESS*4); }
      brightness = value;
    } 
    if (mode == 1 && colorcycle == 0) {
      if (value < 0) { value = 0; myEnc.write(0*4); }
      if (value > (TEMPERATURES_COUNT-1)) { value = (TEMPERATURES_COUNT-1); myEnc.write((TEMPERATURES_COUNT-1)*4); }
      temperature = value;
    }
    if (!colorcycle) {
      RgbColor color;
      color = HtmlColor(temperatures_rgb[temperature]);
      RgbColor result = RgbColor::LinearBlend(RgbColor(0,0,0), color, ((float)brightness)/MAX_BRIGHTNESS);
      strip.ClearTo(result);
      strip.Show();
    }
    if (mode == 0 && colorcycle == 1) {
      for(int i=0; i < PixelCount; i++) {
        float c = ((float)i) / ((float)PixelCount);
        HslColor color = HslColor(c, 1.0f, ((float)brightness)/2.0f/MAX_BRIGHTNESS);
        strip.SetPixelColor(i, color);
      }
    }
    if (mode == 1 && colorcycle == 1) {
      if (value < 0) { value = 0; myEnc.write(0*4); }
      if (value > (MAX_SPEED-1)) { value = (MAX_SPEED-1); myEnc.write((MAX_SPEED-1)*4); }
      speed = value;
    }

  }
  
  if (!digitalRead(10)) {
    
    if (mode == 0) {
      mode = 1;
      Serial.println("change to mode 1");
    } else {
      mode = 0;
      Serial.println("change to mode 0");
    }
    
    uint32_t start = millis();
    Serial.println("click"); 
    while (!digitalRead(10));
    uint32_t stop = millis();
    if (stop - start > 2000) {
      Serial.println("Color cycle!");
      colorcycle = 1;
      mode = 0;
      for(int i=0; i < PixelCount; i++) {
        float c = ((float)i) / ((float)PixelCount);
        HslColor color = HslColor(c, 1.0f, ((float)brightness)/2.0f/MAX_BRIGHTNESS);
        strip.SetPixelColor(i, color);
      }
      strip.Show();
    } else
    if (stop - start > 1000) {
      EEPROM.write(0, ((uint8_t)brightness));
      EEPROM.write(1, ((uint8_t)temperature));
      Serial.println("Saved!");
      for (int i = 0; i < 10; i++) {
        digitalWrite(LED_BUILTIN, LOW);
        delay(100);
        digitalWrite(LED_BUILTIN, HIGH);
        delay(100);
      }
      mode = 0;
    }
    
    if (mode == 0) {
      myEnc.write(brightness*4);
      digitalWrite(LED_BUILTIN, HIGH);
    }
    if (mode == 1 && colorcycle == 0) {
      myEnc.write(temperature*4);
      digitalWrite(LED_BUILTIN, LOW);
    }
    if (mode == 1 && colorcycle == 1) {
      myEnc.write(speed*4);
      digitalWrite(LED_BUILTIN, LOW);
    }
    
  }

  if ( (colorcycle == 1) && ((millis() - last_millis) > (speed * 2)) ) {
    strip.RotateRight(1);
    strip.Show();
    last_millis = millis();
  }
  frame++;
  
}
