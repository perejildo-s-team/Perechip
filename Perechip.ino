// Example testing sketch for various DHT humidity/temperature sensors
// Written by ladyada, public domain

// REQUIRES the following Arduino libraries:
// - DHT Sensor Library: https://github.com/adafruit/DHT-sensor-library
// - Adafruit Unified Sensor Lib: https://github.com/adafruit/Adafruit_Sensor

#include "DHT.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include ".\config.h"

const int AirValue = 620;   
const int WaterValue = 310; 
int soilMoistureValue = 0;
int soilMoisturePercent = 0;

WiFiClient client;
PubSubClient client_esp(client);

#define DHTPIN D2     // Digital pin connected to the DHT sensor
// Feather HUZZAH ESP8266 note: use pins 3, 4, 5, 12, 13 or 14 --
// Pin 15 can work but DHT must be disconnected during program upload.

#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

DHT dht(DHTPIN, DHTTYPE);

void callbacka(char *topic, byte *payload, unsigned int length){
  for (int i = 0; i < length; i++){
    Serial.print((char) payload[i]);
  }
}
void setup() {
  Serial.begin(9600);
  Serial.println(F("DHTxx test!"));
  pinMode(A0, OUTPUT);

  dht.begin();

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid,passwd);

  while(WiFi.status() != WL_CONNECTED){
    delay(1000);
  }
  String client_id = "Pereid";

  client_esp.setServer(mqtt_ip,mqtt_port);

  while (!client_esp.connected()){
    client_esp.connect(client_id.c_str(),user,pass);
    delay(1000);
  }
  client_esp.setCallback(callbacka);
  client_esp.subscribe("hackeps/GF");
}


void loop() {
  // Wait a few seconds between measurements.
  client_esp.loop();
  delay(2000);

  soilMoistureValue = analogRead(A0);
  soilMoisturePercent = map(soilMoistureValue, AirValue, WaterValue, 0, 100);

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

  StaticJsonDocument<256> doc;
  doc["id"] = DHTTYPE;
  doc["air_humidity"] = h;
  doc["temperature"] = hic;
  doc["soil_humidity"] = soilMoistureValue;
  char buffer[256];
  serializeJson(doc, buffer);
  client_esp.publish("hackeps/GF", buffer);
  
  // Serial.print(F("Humidity: "));
  // Serial.print(h);
  // Serial.print(F("%  Temperature: "));
  // Serial.print(t);
  // Serial.print(F("°C "));
  // Serial.print(f);
  // Serial.print(F("°F  Heat index: "));
  // Serial.print(hic);
  // Serial.print(F("°C "));
  // Serial.print(hif);
  // Serial.println(F("°F"));
}