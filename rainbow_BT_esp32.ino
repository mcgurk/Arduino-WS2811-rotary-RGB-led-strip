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

#define LED_BUILTIN 2

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
}

void loop() {
  if (Serial.available()) {
    SerialBT.write(Serial.read());
  }
  if (SerialBT.available()) {
    String S = SerialBT.readStringUntil('\n');
    Serial.println(S);
    
    // Deserialize the JSON document
    DeserializationError error = deserializeJson(doc, S);
  // Test if parsing succeeds.
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return;
  }
  int test = doc["e"];
  Serial.println(test);
  
    char s[100];
    S.toCharArray(s, 99);
    /*for (int i=0; i < sizeof(S)+5; i++) {
      Serial.println(s[i], DEC);
    }*/
    //char c = SerialBT.read();
    char c = s[0];
    //Serial.write(c);
    if (c == 'a') digitalWrite(LED_BUILTIN, HIGH);
    if (c == 'b') digitalWrite(LED_BUILTIN, LOW);
    if (c == '*') {
      //char o;
      //if (digitalRead(LED_BUILTIN)) o = 'a'; else o = 'b';
      //if (digitalRead(LED_BUILTIN)) doc["led"] = 1; else doc["led"] = 0;
      if (digitalRead(LED_BUILTIN)) doc["led"] = "true"; else doc["led"] = "false";
      //SerialBT.write(o);
      //SerialBT.print(" ping\n");
      //SerialBT.print("{ \"a\" : 0, \"b\" : 1 } \n");
      serializeJson(doc, SerialBT); SerialBT.write('\n');
      //Serial.write("ping");

    }
  }
  delay(20);
  // set the brightness on LEDC channel 0
  ledcAnalogWrite(LEDC_CHANNEL_0, 10);
}
