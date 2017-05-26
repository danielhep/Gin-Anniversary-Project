#define FASTLED_ALLOW_INTERRUPTS 0
#include <Switch.h>
#include <FastLED.h>

#include <ESP8266WiFi.h>          //ESP8266 Core WiFi Library (you most likely already have this in your sketch)

#include <DNSServer.h>            //Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h>     //Local WebServer used to serve the configuration portal
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic

#include <BlynkSimpleEsp8266.h>

#include <ArduinoJson.h>

#define LED_PIN     D4

#define L_PIN     D6
#define MID_PIN     D7
#define R_PIN     D8

#define NUM_LEDS    30
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB

#define BLYNK_AUTH "e578fc2140834f858aa4b48306c9def7"

CRGBArray<NUM_LEDS> leds;

bool nightMode = true;

#define UPDATES_PER_SECOND 30

DEFINE_GRADIENT_PALETTE( daniel_gp ) {
  0,     51,  51,  255,
  64,   0,  255,  50,
  128,   0, 255,  0,
  255,   100, 51, 200
};

DEFINE_GRADIENT_PALETTE( plus_gp ) {
  0,     150,  0,  100,
  128,   255,  0,  0,
  255,   150 , 0, 200
};


DEFINE_GRADIENT_PALETTE( gin_gp ) {
  0,     150,  0,  100,
  128,   255,  0,  0,
  255,   150 , 0, 200
};


DEFINE_GRADIENT_PALETTE( night_gp ) {
  0,     255,  0,  0,
  128,   0,  0,  0,
  255, 255, 0, 0
};

// Gradient palette "bhw2_47_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/bhw/bhw2/tn/bhw2_47.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 48 bytes of program space.

DEFINE_GRADIENT_PALETTE( bhw2_47_gp ) {
    0,  78, 43,205,
   22,   1,111,114,
   48, 121,255,125,
   56,  38,191,122,
   79,  46, 93,119,
  112,  78, 43,205,
  135,  78, 43,205,
  165,  46, 93,119,
  193,  38,191,122,
  204, 121,255,125,
  226,   1,111,114,
  255,  78, 43,205};


// Gradient palette "es_landscape_24_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/es/landscape/tn/es_landscape_24.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 24 bytes of program space.

DEFINE_GRADIENT_PALETTE( es_landscape_24_gp ) {
  0,  27, 91,  0,
  38,  94, 255,  0,
  63, 142, 255, 21,
  68, 173, 244, 252,
  127,  10, 164, 156,
  255,   5, 68, 66
};


// Gradient palette "es_rosa_32_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/es/rosa/tn/es_rosa_32.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 28 bytes of program space.

DEFINE_GRADIENT_PALETTE( es_rosa_32_gp ) {
    0,  86,  0,  1,
   51,  86,  0,  1,
   58, 121,  1, 61,
   84, 121,  1, 61,
  109, 121,  1, 61,
  117,  86,  0,  1,
  255, 121,  1, 61};


CRGBPalette16 danielPalette = es_landscape_24_gp;
CRGBPalette16 plusPalette = es_rosa_32_gp;
CRGBPalette16 ginPalette = bhw2_47_gp;
CRGBPalette16 nightPalette = night_gp;

Switch MIDButton = Switch(MID_PIN, INPUT, HIGH);
Switch LButton = Switch(L_PIN, INPUT, HIGH);
Switch RButton = Switch(R_PIN, INPUT, HIGH);

int brightness;
int nighttimeBrightness = 3;
int daytimeBrightness = 190;

int nightTimeStart = 19;
int nightTimeEnd = 8;

void setup() {
  WiFiManager wifiManager;

  delay( 3000 ); // power-up safety delay
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness( 255 );

  fill_solid( leds, NUM_LEDS, CRGB::Black );
  FastLED.show();

  Serial.begin(9600);

  pinMode(R_PIN, INPUT);
  pinMode(MID_PIN, INPUT);
  pinMode(L_PIN, INPUT);

  wifiManager.setConfigPortalTimeout(60);
  wifiManager.autoConnect("G+D");

  Blynk.config(BLYNK_AUTH);
  Blynk.run();

  startupEffect();
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    Blynk.run();
    processNetwork();
  }

  //  Serial.println(millis());
  if (nightMode) {
    brightness = nighttimeBrightness;
  } else {
    brightness = daytimeBrightness;
  }

  runLeds();
  detectButtons();
}

void detectButtons() {
  MIDButton.poll();
  LButton.poll();
  RButton.poll();

  if (MIDButton.on()) {
    if (LButton.on()) {
      brightness -= 5;
      Serial.println("pin high");
      if (brightness <= 0)
        brightness = 1;
    }
    if (RButton.on()) {
      brightness += 5;
      Serial.println("pin high");
      if (brightness >= 255)
        brightness = 255;
    }
  }

  if (MIDButton.released()) {
    if (nightMode) {
      Blynk.virtualWrite(V1, brightness);
    } else {
      Blynk.virtualWrite(V0, brightness);
    }
  }

  if (nightMode) {
    nighttimeBrightness = brightness;
  } else {
    daytimeBrightness = brightness;
  }
}

void runLeds() {
  static uint8_t startIndex = 0;
  startIndex = startIndex + 1; /* motion speed */
  FillLEDsFromPalette(startIndex);

  FastLED.show();
  FastLED.delay(1000 / UPDATES_PER_SECOND);
}

void FillLEDsFromPalette(uint8_t colorIndex) {
  if (nightMode) {
    for ( int i = 0; i < 30; i++) {
      leds[i] = ColorFromPalette(nightPalette, colorIndex, brightness);
      colorIndex += 25;
    }
  } else {
    // Serial.println(brightness);
    for ( int i = 0; i < 13; i++) {
      leds[i] = ColorFromPalette(ginPalette, colorIndex, brightness);
      colorIndex += 20;
    }
    for (int i = 13; i < 17; i++ ) {
      leds[i] = ColorFromPalette(plusPalette, colorIndex, brightness);
      colorIndex += 32;
    }
    for ( int i = 17; i < 30; i++) {
      leds[i] = ColorFromPalette(danielPalette, colorIndex, brightness);
      colorIndex += 20;
    }
  }
}

void startupEffect() {
  for (int i = 0; i < NUM_LEDS + 1; i++) {
    fill_rainbow(leds, i, 0, 10);
    FastLED.show();
    FastLED.delay(50);
  }
}

CRGBArray<NUM_LEDS> oldLeds;
void sendPulse(CRGB color) {
  oldLeds = leds;
  for(int i = 0; i < NUM_LEDS + 1; i++) {
    leds[i] = color;
    if(i > 1) {
      leds[i-2] = oldLeds[i-2];
    }
    FastLED.show();
    FastLED.delay(100);
  }
}

// Keep this flag not to re-sync on every reconnection
bool isFirstConnect = true;
BLYNK_CONNECTED() {
  if (isFirstConnect) {
    Blynk.syncAll();
    isFirstConnect = false;
  }
}

BLYNK_WRITE(V1) //Button Widget is writing to pin V1
{
  nighttimeBrightness = param.asInt();
}

BLYNK_WRITE(V0) //Button Widget is writing to pin V1
{
  daytimeBrightness = param.asInt();
}

BLYNK_WRITE(V2) //Button Widget is writing to pin V1
{
  nightTimeStart = param.asInt();
  getTime();
}

BLYNK_WRITE(V3) //Button Widget is writing to pin V1
{
  nightTimeEnd = param.asInt();
  getTime();
}

CRGB pulseColor = CRGB(255,0,0);

BLYNK_WRITE(V4)
{
  if(param.asInt())
    sendPulse(pulseColor);
}


BLYNK_WRITE(V5)
{
  pulseColor = CRGB(param[0].asInt(), param[1].asInt(), param[2].asInt());
}

