#include <NeoPixelBus.h>

const uint16_t PixelCount = 20;
const uint8_t PixelPin = 2;
//char data[1000];
uint16_t maxchars = PixelCount*3 + 1;

#define colorSaturation 128
#define MAX_BRIGHTNESS 128

NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod>* strip = NULL;


RgbColor red(colorSaturation, 0, 0);
RgbColor green(0, colorSaturation, 0);
RgbColor blue(0, 0, colorSaturation);
RgbColor white(colorSaturation);
RgbColor black(0);

HslColor hslRed(red);
HslColor hslGreen(green);
HslColor hslBlue(blue);
HslColor hslWhite(white);
HslColor hslBlack(black);


void setup()
{
    Serial.begin(115200);
    //while (!Serial); // wait for serial attach

    Serial.println();
    Serial.println("Initializing...");
    Serial.flush();

    strip = new NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod>(PixelCount, PixelPin); // and recreate with new count
    strip->Begin();
    strip->Show();

    Serial.println();
    Serial.println("Running...");

    for(uint16_t i = 0; i < PixelCount; i++) {
      float c = ((float)i) / ((float)PixelCount);
      //HslColor color = HslColor(c, 1.0f, ((float)brightness)/2.0f/MAX_BRIGHTNESS);
      HslColor color = HslColor(c, 1.0f, MAX_BRIGHTNESS / 255.0f);
      strip->SetPixelColor(i, color);
    }
    strip->Show();
}


void loop()
{
    /*delay(1000);

    Serial.println("Colors R, G, B, W...");

    // set the colors, 
    // if they don't match in order, you need to use NeoGrbFeature feature
    strip->SetPixelColor(0, red);
    strip->SetPixelColor(1, green);
    strip->SetPixelColor(2, blue);
    strip->SetPixelColor(3, white);
    // the following line demonstrates rgbw color support
    // if the NeoPixels are rgbw types the following line will compile
    // if the NeoPixels are anything else, the following line will give an error
    //strip->SetPixelColor(3, RgbwColor(colorSaturation));
    strip->Show();


    delay(1000);

    Serial.println("Off ...");

    // turn off the pixels
    strip->SetPixelColor(0, black);
    strip->SetPixelColor(1, black);
    strip->SetPixelColor(2, black);
    strip->SetPixelColor(3, black);
    strip->Show();*/

    strip->RotateRight(1);
    strip->Show();
    
    pollSerial();

    delay(100);
}


void pollSerial() {
  if (!Serial.available()) return;
  //uint16_t cnt = Serial.readBytes(data, maxchars);
  uint16_t cnt = Serial.readBytes(strip->Pixels(), maxchars);
  //data[cnt] = '\0';
  Serial.print(cnt); //Serial.print(":"); Serial.println(data);
  strip->Dirty();
  strip->Show();
}
