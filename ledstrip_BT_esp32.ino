#include <ArduinoJson.h>

//This example code is in the Public Domain (or CC0 licensed, at your option.)
//By Evandro Copercini - 2018
//
//This example creates a bridge between Serial and Classical Bluetooth (SPP)
//and also demonstrate that SerialBT have the same functionalities of a normal Serial

#include "BluetoothSerial.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

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
  //float speed;
  //float periods;
  //uint8_t fading;
  //RgbColor color;
  //float phase;
  //uint16_t checksum;
} effect;

// fade LED PIN
#define LED_PIN            4
// use first channel of 16 channels (started from zero)
#define LEDC_CHANNEL_0     0
// use 13 bit precission for LEDC timer
#define LEDC_TIMER_13_BIT  13
// use 5000 Hz as a LEDC base frequency
#define LEDC_BASE_FREQ     5000
// Arduino like analogWrite
// value has to be between 0 and valueMax
void ledcAnalogWrite(uint8_t channel, uint32_t value, uint32_t valueMax = 255) {
  // calculate duty, 8191 from 2 ^ 13 - 1
  uint32_t duty = (8191 / valueMax) * min(value, valueMax);
  // write duty to LEDC
  ledcWrite(channel, duty);
}

void setup() {
  Serial.begin(115200);
  SerialBT.begin("ESP32test"); //Bluetooth device name
  Serial.println("The device started, now you can pair it with bluetooth!");
  pinMode(LED_BUILTIN, OUTPUT);

  // Setup timer and attach timer to a led pin
  ledcSetup(LEDC_CHANNEL_0, LEDC_BASE_FREQ, LEDC_TIMER_13_BIT);
  ledcAttachPin(LED_PIN, LEDC_CHANNEL_0);

  effect.mode = MODE_STATIC;
  effect.onOff = true;
  effect.brightness = 128;
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
    /*for (int i=0; i < sizeof(S)+5; i++) {
      Serial.println(s[i], DEC);
    }*/
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
        serializeJson(doc, SerialBT); SerialBT.write('\n');
        break;
      default:
        parseEffect();
        updateEffect();
    }
  }

  if (effect.mode > 1 && effect.onOff == true) updateEffect();
  delay(20);

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
  IFKEY("onOff") effect.onOff = doc["onOff"];
  IFKEY("mode") effect.mode = doc["mode"];
}

void updateEffect() {
  if (effect.onOff) {
    digitalWrite(LED_BUILTIN, HIGH);
    switch (effect.mode) {
      case MODE_STATIC:
        ledcAnalogWrite(LEDC_CHANNEL_0, effect.brightness);
        break;
      case MODE_RAINBOW:
        //updateRainbow();
        uint8_t a = (millis() >> 1);
        ledcAnalogWrite(LEDC_CHANNEL_0, a);
        break;
    }
  } else {
    digitalWrite(LED_BUILTIN, LOW);
  }
}
