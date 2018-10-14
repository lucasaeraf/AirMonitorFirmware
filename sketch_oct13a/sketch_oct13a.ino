#include <BearSSLHelpers.h>
#include <CertStoreBearSSL.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiAP.h>
#include <ESP8266WiFiGeneric.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WiFiScan.h>
#include <ESP8266WiFiSTA.h>
#include <ESP8266WiFiType.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <WiFiClientSecureAxTLS.h>
#include <WiFiClientSecureBearSSL.h>
#include <WiFiServer.h>
#include <WiFiServerSecure.h>
#include <WiFiServerSecureAxTLS.h>
#include <WiFiServerSecureBearSSL.h>
#include <WiFiUdp.h>

#include "DHTesp.h"
#include <ESP8266WiFi.h>
#define DHTPIN D2
#define DSMPIN D1

unsigned long duration;
unsigned long starttime;
unsigned long sampletime_ms = 30000;
unsigned long lowpulseoccupancy = 0;

const char* ssid = "ANA RAQUEL";
const char* password = "gato1234";
const char* host = "api.thingspeak.com/";

float ratio = 0;
float concentration = 0;
float temp;
float humidity;
DHTesp dht;

void connectWiFi(void);

void connectWiFi(void){
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void setup() {
  Serial.begin(115200);
  pinMode(DSMPIN, INPUT);
  connectWiFi();
  dht.setup(DHTPIN, DHTesp::DHT22);
  starttime = millis();
}

void loop() {
  duration = pulseIn(DSMPIN, LOW);
  lowpulseoccupancy += duration;

  if ((millis()-starttime) > sampletime_ms)//if the sampel time == 30s
  {
    
    float humidity = dht.getHumidity();
    float temp = dht.getTemperature();
    Serial.print("Umidade: ");
    Serial.print(temp);
    Serial.print(" %\t");
    Serial.print("Temperatura: ");
    Serial.print(humidity);
    Serial.println(" *C");
    Serial.print("Low Pulse Occupancy: ");
    Serial.print(lowpulseoccupancy);
    Serial.print("\n");
    ratio = lowpulseoccupancy/(sampletime_ms)*10.0;  // Integer percentage 0=>100
    concentration = 0.001915*pow(ratio,2) + 0.09522*ratio - 0.04884; // using spec sheet curve
    Serial.print("concentration = ");
    Serial.print(concentration);
    Serial.print(" mg/m^3\n");
    
    lowpulseoccupancy = 0;
    starttime = millis();
  }
}
