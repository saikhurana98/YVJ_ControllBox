/*************************************************************
  This is a DEMO. You can use it only for development and testing.
  You should open Setting.h and modify General options.

  If you would like to add these features to your product,
  please contact Blynk for Businesses:

                   http://www.blynk.io/

 *************************************************************/

// #define USE_SPARKFUN_BLYNK_BOARD    // Uncomment the board you are using
// #define USE_NODE_MCU_BOARD // Comment out the boards you are not using
//#define USE_WITTY_CLOUD_BOARD
#define USE_CUSTOM_BOARD // For all other ESP8266-based boards -
// see "Custom board configuration" in Settings.h

#define APP_DEBUG // Comment this out to disable debug prints

// #define BLYNK_SSL_USE_LETSENCRYPT // Comment this out, if using public Blynk Cloud

#define BLYNK_PRINT Serial
//#define BLYNK_DEBUG

#include "BlynkProvisioning.h"
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <ESP8266mDNS.h> // Include the mDNS library
#include <ezButton.h>

// Pinout
#define led_strip_r D7
#define led_strip_g D8
#define led_strip_b D6

#define relay_1 D0
#define relay_2 D1
#define relay_3 D5
#define relay_4 3 // RX

#define relay_input_1 D2
#define relay_input_2 D3

// APP Layout
#define ledPwr V0
#define zRGBra V1
#define blynk_switch_1 V2
#define blynk_switch_2 V3
#define blynk_switch_3 V4
#define blynk_switch_4 V5
#define ledBright V6
#define slideR V7
#define slideG V8
#define slideB V9

struct color
{
  uint32 r;
  uint32 g;
  uint32 b;
} typedef color;

// Global Variables
color prev_color;
color curr_color;
uint8_t curr_bright = 100;
bool relay_1_state = false;
bool relay_2_state = false;
bool relay_3_state = false;
bool relay_4_state = false;
bool pwrState = true;

// Global Object Instances
BlynkTimer timer;
ezButton relay_button_1(relay_input_1);
ezButton relay_button_2(relay_input_2);
// Function Definitions
void write_color(color rgb);
void setColor(color rgb, uint16_t brightness);

void setup()
{
  delay(100);
  Serial.begin(115200);
  pinMode(led_strip_r, OUTPUT);
  pinMode(led_strip_g, OUTPUT);
  pinMode(led_strip_b, OUTPUT);
  pinMode(relay_1, OUTPUT);
  pinMode(relay_2, OUTPUT);
  pinMode(relay_3, OUTPUT);
  pinMode(relay_4, OUTPUT);

  digitalWrite(relay_1, HIGH);
  digitalWrite(relay_2, HIGH);
  digitalWrite(relay_3, HIGH);
  digitalWrite(relay_4, HIGH);

  relay_button_1.setDebounceTime(50); // set debounce time to 50 milliseconds
  relay_button_2.setDebounceTime(50); // set debounce time to 50 milliseconds

  BlynkProvisioning.begin();

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
    {
      type = "sketch";
    }
    else
    { // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR)
    {
      Serial.println("Auth Failed");
    }
    else if (error == OTA_BEGIN_ERROR)
    {
      Serial.println("Begin Failed");
    }
    else if (error == OTA_CONNECT_ERROR)
    {
      Serial.println("Connect Failed");
    }
    else if (error == OTA_RECEIVE_ERROR)
    {
      Serial.println("Receive Failed");
    }
    else if (error == OTA_END_ERROR)
    {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
}

void loop()
{
  // This handles the network and cloud connection
  BlynkProvisioning.run();
  ArduinoOTA.handle();
  relay_button_1.loop();
  relay_button_2.loop();

  if (!MDNS.begin("esp8266"))
  {
    Serial.println("Error setting up MDNS responder!");
  }


  if (relay_button_1.isReleased())
  {
    if (!relay_1_state)
    {
      digitalWrite(relay_1, LOW);
      relay_1_state = true;
    }
    else
    {
      digitalWrite(relay_1, HIGH);
      relay_1_state = false;
    }
    Blynk.virtualWrite(blynk_switch_1, relay_1_state);
  }
  if (relay_button_2.isReleased())
  {
    if (!relay_2_state)
    {
      digitalWrite(relay_2, LOW);
      relay_2_state = true;
    }
    else
    {
      digitalWrite(relay_2, HIGH);
      relay_2_state = false;
    }
    Blynk.virtualWrite(blynk_switch_2, relay_2_state);
  }
}

BLYNK_WRITE(ledPwr)
{
  if (!param.asInt())
  {
    Blynk.virtualWrite(ledBright,0);
    setColor(curr_color, 0);
    pwrState = false;
    return;
  }
    pwrState = true;
    Blynk.virtualWrite(ledBright,curr_bright);
  setColor(curr_color, curr_bright);
}

BLYNK_WRITE(zRGBra)
{
  color rgb;
  // get a RED channel value
  rgb.r = param[0].asInt();
  // get a GREEN channel value
  rgb.g = param[1].asInt();
  // get a BLUE channel value
  rgb.b = param[2].asInt();

  write_color(rgb);
}

BLYNK_WRITE(blynk_switch_1)
{
  relay_1_state = (bool)param.asInt();
  digitalWrite(relay_1, !relay_1_state);
}
BLYNK_WRITE(blynk_switch_2)
{
  relay_2_state = (bool)param.asInt();
  digitalWrite(relay_2, !relay_2_state);
}
BLYNK_WRITE(blynk_switch_3)
{
  relay_3_state = (bool)param.asInt();
  digitalWrite(relay_3, !relay_3_state);
}
BLYNK_WRITE(blynk_switch_4)
{
  relay_4_state = (bool)param.asInt();
  digitalWrite(relay_4, !relay_4_state);
}

BLYNK_WRITE(ledBright)
{
  curr_bright = param.asInt();
  setColor(curr_color, curr_bright);
}

BLYNK_WRITE(slideR)
{
  curr_color.r = param.asInt();
  Blynk.virtualWrite(zRGBra, curr_color.r, curr_color.g, curr_color.b);
  setColor(curr_color, curr_bright);
}
BLYNK_WRITE(slideG)
{
  curr_color.g = param.asInt();
  Blynk.virtualWrite(zRGBra, curr_color.r, curr_color.g, curr_color.b);
  setColor(curr_color, curr_bright);
}
BLYNK_WRITE(slideB)
{
  curr_color.b = param.asInt();
  Blynk.virtualWrite(zRGBra, curr_color.r, curr_color.g, curr_color.b);
  setColor(curr_color, curr_bright);
}


void setColor(color rgb, uint16_t brightness)
{
  if (pwrState)
  {
    rgb.r = (uint32_t)rgb.r * brightness / 100;
    rgb.g = (uint32_t)rgb.g * brightness / 100;
    rgb.b = (uint32_t)rgb.b * brightness / 100;

    // write_color(rgb);
    while (prev_color.r != rgb.r || prev_color.g != rgb.g || prev_color.b != rgb.b)
    {
      if (prev_color.r < rgb.r)
        prev_color.r += 1;
      if (prev_color.r > rgb.r)
        prev_color.r -= 1;

      if (prev_color.g < rgb.g)
        prev_color.g += 1;
      if (prev_color.g > rgb.g)
        prev_color.g -= 1;

      if (prev_color.b < rgb.b)
        prev_color.b += 1;
      if (prev_color.b > rgb.b)
        prev_color.b -= 1;

      write_color(prev_color);
    }
  }
}

void write_color(color rgb)
{
  analogWrite(led_strip_r, rgb.r);
  analogWrite(led_strip_g, rgb.g);
  analogWrite(led_strip_b, rgb.b);
}