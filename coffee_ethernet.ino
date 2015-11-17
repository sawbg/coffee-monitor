/*
 * WiFi Coffee Thermometer
 * (c) 2015 Andrew Wisner
 * In the future, remove necessity for floating point numbers.
 * Floats are not natively supported, and these slow down the code!
*/

// headers
#include "coftypes.h"  // type definitions
#include <Ethernet.h>
#include <LiquidCrystal.h>
#include <SPI.h>
#include <string.h>

// constants
#define ATD_PIN 5
#define ATD_MAX 1023
#define FRESH_TEMP_DELTA 5
#define FRESH_TIME_DELTA 60000
#define MAX_VOLTAGE 3.58
#define R_DIV 9770
#define SERVER_PORT 8080
#define THERM_BETA 4204
#define THERM_DISS 0.0065
#define THERM_R_NAUGHT 50000
#define THERM_KELVIN 298.15

//#define NO_ETHERNET

// variable initialization
unsigned int brewTimeDelta;
unsigned long lastBrewTime;
String json;
InfoBox box;
InfoBox chkpt;
EthernetServer server(SERVER_PORT);
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);  // args based on pin connnection

void setup() {
  // initialize serial port and wait
  Serial.begin(9600);  // (baud rate)
  while (!Serial);

  // initialize LCD screen
  lcd.begin(16, 2);  // (characters per line, lines)

// INSERT NO_ETHERNET error message here
  
  IPAddress ipAddress(192, 168, 1, 22);
  byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
  Ethernet.begin(mac, ipAddress);
  server.begin();
  
  lastBrewTime = 0;
}

void loop() {
  // READ
  EthernetClient client = server.available();
  box.atdVal = analogRead(ATD_PIN);
  box.time = millis();

  // THINK
  box.resistance = resistance(box.atdVal);
  box.voltage = voltage(box.atdVal);
  box.temperature = temperature(box.resistance, box.voltage);
  brewTimeDelta = (box.time - lastBrewTime) / 60000;

  // determine status
  if (box.temperature - chkpt.temperature == FRESH_TEMP_DELTA) {
    lastBrewTime = box.time;
    box.status = BREWING;
  } else if (brewTimeDelta > FRESH) {
    box.status = FRESH;
  } else if (brewTimeDelta > GOOD) {
    box.status = GOOD;
  } else if (brewTimeDelta > FAIR) {
    box.status = FAIR;
  } else if (brewTimeDelta > OLD) {
    box.status = OLD;
  } else if (brewTimeDelta > STALE) {
    box.status = STALE;
  }

  // DO
  // updates checkpoint
  chkpt = ((box.time - chkpt.time) / FRESH_TIME_DELTA > 0) ? chkpt : box;

  #ifndef NO_ETHERNET
  // send JSON
  json = toJson(box);
  Serial.println(json);
  
  if (client && client.connected() && client.available()) {
    client.println(json);
    Serial.println("JSON just sent...");
  }
  
  delay(1000);  // gives client time to receive JSON and provides constant loop time
  if (client) client.stop();
  
  #else
  //Serial.println(toJson(box));
  delay(3000);
  #endif
}

void print(char mesg0[], char mesg1[]) {
  Serial.println(mesg0);
  Serial.println(mesg1);

  lcd.setCursor(0, 0);  // first column, first row
  lcd.print(mesg0);
  lcd.setCursor(0, 1);  // first column, second row
  lcd.print(mesg1);
}

float resistance(unsigned int atdVal) {
  return (float)R_DIV * (float)voltage(atdVal) / ((float)MAX_VOLTAGE - (float)voltage(atdVal));
}

float temperature(float resistance, float voltage) {
  return 1./(1./(float)THERM_KELVIN + (log(resistance)-log(THERM_R_NAUGHT))/(float)THERM_BETA) 
    - pow(voltage, 2) / ((float)THERM_DISS * resistance) - 273.15 - 6;
}

String toJson(InfoBox box) {
  return "{\"temperature\": " + String((int)box.temperature) + ", \"status\": \""
    + toString(box.status) + "\", \"age\": " + String(brewTimeDelta) + "}";
}

String toString(Status status) {
  String str = "";
  
  switch(status) {
    case BREWING:
      str = "BREWING";
      break;
    case FRESH:
      str = "FRESH";
      break;
    case GOOD:
      str = "GOOD";
      break;
    case FAIR:
      str = "FAIR";
      break;
    case OLD:
      str = "OLD";
      break;
    case STALE:
      str = "STALE";
      break;
    default:
      str = "ERROR";
      break;
  }
  
  return str;
}

float voltage(unsigned int atdVal) {
  return MAX_VOLTAGE * (float)atdVal / (float)ATD_MAX;
}
