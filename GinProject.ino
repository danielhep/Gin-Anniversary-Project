#define FASTLED_ALLOW_INTERRUPTS 0
#include <Switch.h>
#include <FastLED.h>

#include <ESP8266WiFi.h>          //ESP8266 Core WiFi Library (you most likely already have this in your sketch)

#include <DNSServer.h>            //Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h>     //Local WebServer used to serve the configuration portal
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic

#define LED_PIN     D4

#define L_PIN     D6
#define MID_PIN     D7
#define R_PIN     D8

#define NUM_LEDS    30
#define STARTING_BRIGHTNESS  200
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
// CRGB leds[NUM_LEDS];
CRGBArray<NUM_LEDS> leds;

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

CRGBPalette16 danielPalette = daniel_gp;
CRGBPalette16 plusPalette = plus_gp;

int brightness = STARTING_BRIGHTNESS;

Switch MIDButton = Switch(MID_PIN, INPUT, HIGH);
Switch LButton = Switch(L_PIN, INPUT, HIGH);
Switch RButton = Switch(R_PIN, INPUT, HIGH);

void setup() {
  WiFiManager wifiManager;

  delay( 3000 ); // power-up safety delay
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness( brightness );

  Serial.begin(9600);

  pinMode(R_PIN, INPUT);
  pinMode(MID_PIN, INPUT);
  pinMode(L_PIN, INPUT);
}

void loop() {
  runLeds();
  detectButtons();
}

void detectButtons() {
  MIDButton.poll();
  LButton.poll();
  RButton.poll();

  if (MIDButton.doubleClick()) {
    Serial.println("Double clicked!");
    if (brightness > 60) {
      brightness = 15;
    } else if (brightness < 200) {
      brightness = 200;
    }
  }
  
    if (MIDButton.on()) {
      brightness += 5;
      Serial.println("pin high");
      if (brightness == 255)
        brightness = 5;
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
  // Serial.println(brightness);
  for ( int i = 0; i < 13; i++) {
    leds[i] = ColorFromPalette(CloudColors_p, colorIndex, brightness);
    colorIndex += 5;
  }
  for (int i = 13; i < 17; i++ ) {
    leds[i] = ColorFromPalette(plusPalette, colorIndex, brightness);
    colorIndex += 32;
  }
  for ( int i = 17; i < 30; i++) {
    leds[i] = ColorFromPalette(danielPalette, colorIndex, brightness);
    colorIndex += 10;
  }
}

void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());

  Serial.println(myWiFiManager->getConfigPortalSSID());
}
