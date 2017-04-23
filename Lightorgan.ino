
// https://github.com/Makuna/NeoPixelBus/wiki/NeoPixelBus-object-API
// https://github.com/Makuna/NeoPixelBus/wiki/HslColor-object-API


#define DEBUG

#include <NeoPixelBus.h>

//const uint16_t PIXELCOUNT = 150; // this example assumes 4 pixels, making it smaller will cause a failure
#define PIXELCOUNT 150
const uint8_t PixelPin = 2;  // make sure to set this to the correct pin, ignored for Esp8266

NeoPixelBus<NeoRgbFeature, Neo800KbpsMethod> strip(PIXELCOUNT, PixelPin);

RgbColor bgbuf[PIXELCOUNT];
RgbColor fgbuf[PIXELCOUNT];

//float traveller_pos = 0;
uint16_t traveller_pos = 0;
uint16_t traveller_speed = 30;
uint8_t traveller_direction = 1;
uint8_t traveller = 1;
RgbColor traveller_color = RgbColor(255, 0, 0);

void setup() {
  Serial.begin(115200);
  while (!Serial); // wait for serial attach

  Serial.println();
  Serial.println("Initializing...");
  Serial.flush();


  // this resets all the neopixels to an off state
  strip.Begin();
  strip.Show();

  Serial.println();
  Serial.println("Running...");

  setRainbow(0.1);
  //showBuffer();

  Serial.println("Setup end");
}

void setRainbow(float brightness) {
  float b = brightness / 2.0;
  for (uint16_t i = 0; i < PIXELCOUNT; i++) {
    float n = ((float)i)/PIXELCOUNT;
    //strip.SetPixelColor(i, HslColor(n, 1.0, 0.5));
    bgbuf[i] = RgbColor(HslColor(n, 1.0, b));
  }
  //strip.Show();
}

void setColor(uint8_t R, uint8_t G, uint8_t B) {
  /*for (uint16_t i = 0; i < PIXELCOUNT; i++) {
    strip.SetPixelColor(i, RgbColor(R, G, B));
  }*/
  //strip.ClearTo(RgbColor(R, G, B));
  //strip.Show();
  for (uint16_t i = 0; i < PIXELCOUNT; i++) {
    HslColor c = RgbColor(R, B, G);
    if (c.L > 0.1) c.L = 0.1;
    bgbuf[i] = RgbColor(c);
    //bgbuf[i] = RgbColor(R, B, G);
  }
}

void setRandom() {
  for (uint16_t i = 0; i < PIXELCOUNT; i++) {
    float n = ((float)i)/PIXELCOUNT;
    //strip.SetPixelColor(i, HslColor(((float)random(256))/255.0, 1.0, 0.5));
    HslColor c = HslColor(((float)random(256))/255.0, 1.0, 0.5);
    bgbuf[i] = RgbColor(c);
  }
  //strip.Show();
}

void setBlock(uint8_t b) {
  uint16_t l = PIXELCOUNT / 14;
  uint16_t o = l * b;
  for (uint16_t i = 0; i < l; i++) {
    //strip.SetPixelColor(i + o, RgbColor(255, 255, 255));
    fgbuf[i+o] = RgbColor(255, 255, 255);
  }
}

/*void showBuffer() {
  for (uint16_t i = 0; i < PIXELCOUNT; i++) {
    strip.SetPixelColor(i, buf[i]);
  }
  strip.Show();
}*/

void fadeFgBuffer() {
  for (uint16_t i = 0; i < PIXELCOUNT; i++) {
    //if (buf[i] => 1) buf[i] = buf[i] - 1;
    HslColor c = fgbuf[i];
    //c.L = c.L / 1.5;
    c.L = c.L / 1.3;
    fgbuf[i] = RgbColor(c);
  }
}

void loop() {

  /*for (uint16_t i = 0; i < PIXELCOUNT; i++) {
    buf[i] = bgbuf[i];
  }*/

  while (Serial.available() > 0) {
    uint8_t c = Serial.read();
    Serial.print("0x"); Serial.println(c, HEX);
    setData(c);
  }

  //rotate bgbuf
  for (uint16_t i = 0; i < PIXELCOUNT-1; i++) {
    bgbuf[i] = bgbuf[i+1];
  }
  bgbuf[PIXELCOUNT-1] = bgbuf[0];

  if (traveller_direction == 0)
    traveller_pos = (millis() / traveller_speed) % PIXELCOUNT;
  else
    traveller_pos = (PIXELCOUNT-1) - ((millis() / traveller_speed) % PIXELCOUNT);
  if(traveller) fgbuf[traveller_pos] = traveller_color;
  //strip.RotateLeft(1);
  //strip.Show();
  fadeFgBuffer();

  for (uint16_t i = 0; i < PIXELCOUNT; i++) {
    uint8_t R = fgbuf[i].R;
    uint8_t G = fgbuf[i].G;
    uint8_t B = fgbuf[i].B;
    if (R < 2 && G < 2 && B < 2)
    //if (R+G+B < 7)
      strip.SetPixelColor(i, bgbuf[i]);
    else
      strip.SetPixelColor(i, fgbuf[i]);
  }
    
  strip.Show();
  //showBuffer();
  delay(10);

}

inline void setData(uint8_t c) {
  switch (c) {
    case 'i':
      setColor(255, 0, 0);
      break;
    case 'o':
      setColor(0, 255, 0);
      break;
    case 'p':
      setColor(0, 0, 255);
      break;
    case 0xE5: //'å':
      setColor(255, 255, 0);
      break;
    case 0xE4: //'ä':
      setColor(0, 255, 255);
      break;
    case '+':
      setColor(255, 0, 255);
      break;
    case 'l':
      setRainbow(1.0);
      break;
    case 0xF6: //'ö':
      setRandom();
      break;
    case '.':
      setColor(0, 0, 0);
      break;
    case 'q':
      setBlock(0);
      break;
    case 'a':
      setBlock(1);
      break;
    case 'w':
      setBlock(2);
      break;
    case 's':
      setBlock(3);
      break;
    case 'e':
      setBlock(4);
      break;
    case 'd':
      setBlock(5);
      break;
    case 'r':
      setBlock(6);
      break;
    case 'f':
      setBlock(7);
      break;
    case 't':
      setBlock(8);
      break;
    case 'g':
      setBlock(9);
      break;
    case 'y':
      setBlock(10);
      break;
    case 'h':
      setBlock(11);
      break;
    case 'u':
      setBlock(12);
      break;
    case 'j':
      setBlock(13);
      break;
    case '<':
      traveller_direction = 0;
      break;
    case '>':
      traveller_direction = 1;      
      break;
    case 'z':
      traveller = 1;
      break;
    case 'x':
      traveller = 0;      
      break;
    default:
      break;
  }
}
