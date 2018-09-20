// https://github.com/Makuna/NeoPixelBus
// https://github.com/Makuna/NeoPixelBus/wiki/NeoPixelBus-object-API
// https://github.com/Makuna/NeoPixelBus/wiki/HslColor-object-API

#include <NeoPixelBus.h>
#include <MemoryUsage.h>

#define MAX_PIXELS 600

uint16_t PixelCount = 150;
//uint16_t PixelCount = MAX_PIXELS;
const uint8_t PixelPin = 2;
//char data[1000];
uint16_t maxchars = PixelCount*3;
//uint8_t data[600*3];
uint16_t frame = 0;
uint32_t micros_start = micros(), micros_end, micros_diff;

//#define colorSaturation 128
#define MAX_BRIGHTNESS 64

//NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod>* strip = NULL;
NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(MAX_PIXELS, PixelPin);

/*RgbColor red(colorSaturation, 0, 0);
RgbColor green(0, colorSaturation, 0);
RgbColor blue(0, 0, colorSaturation);
RgbColor white(colorSaturation);
RgbColor black(0);

HslColor hslRed(red);
HslColor hslGreen(green);
HslColor hslBlue(blue);
HslColor hslWhite(white);
HslColor hslBlack(black);*/


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


void loop() {
  //Serial.println("loop begin");
  //uint32_t s,e,t;
  float b = MAX_BRIGHTNESS / 255.0f;
  #define SPEED 100
  float f = ((float)(frame % SPEED)) / SPEED;
/*  noInterrupts();
  s = micros();*/
  for (uint16_t i = 0; i < PixelCount; i++) {
    float c = ((float)i) / ((float)PixelCount);
    c += f;
    float temp = ((int)c); c -= temp; //take only fractional part
    HslColor color = HslColor(c, 1.0f, b);
    strip.SetPixelColor(i, color);
  }
/*  e = micros();
  interrupts();
  t = e - s;
  Serial.println(t);

  noInterrupts();
  s = micros();*/
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
  if (frame == 1000) frame = 0;
  //delay(100);

    /*Serial.println("------s-----");
    MEMORY_PRINT_START
    MEMORY_PRINT_HEAPSTART
    MEMORY_PRINT_HEAPEND
    MEMORY_PRINT_STACKSTART
    MEMORY_PRINT_END
    MEMORY_PRINT_HEAPSIZE
    Serial.println();
    FREERAM_PRINT;
    Serial.println("------e-----");*/
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
