#include "DHTesp.h"
#include <ESP8266WiFi.h>
#define DHTPIN D2
#define DSMPIN D1
#define DSMPIN25 D4
#define MQ135PIN D3

bool threshold;

unsigned long duration;
unsigned long duration25;
unsigned long starttime;
unsigned long sampletime_ms = 30000;
unsigned long lowpulseoccupancy = 0;
unsigned long lowpulseoccupancy25 = 0;

const char* ssid = "Sidnet 7";
const char* password = "ehoradoshowporra";
const char* server = "api.thingspeak.com";
const String apiKey = "6Q4GC2JNZIQD7Q1R";

float ratio = 0;
float ratio25 = 0;
float concentration = 0;
float concentration25 = 0;
float temp;
float humidity;
DHTesp dht;
WiFiClient client;

void connectWiFi(void);
void postData(String data);

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

void postData(String data){
  //Connect to server
  Serial.print("connecting to ");
  Serial.println(server);
  if (!client.connect(server, 80)) {
    Serial.println("connection failed");
    return;
  }
  Serial.println("Sending data...");
  if(client.connected()){
    client.println("POST /update HTTP/1.1");
    client.println("Host: api.thingspeak.com");
    client.println("Connection: close");
    client.println("X-THINGSPEAKAPIKEY: " + apiKey);
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.print("Content-Length: ");
    client.println(String(data.length()));
    client.println("");
    client.println(data);

    Serial.println("Data POSTed");
    
    // Read all the lines of the reply from server and print them to Serial
    while (client.available()) {
      String line = client.readStringUntil('\r');
      Serial.print(line);
    }
  }
  else Serial.println("An error has ocurred.");
  client.stop();
}
void setup() {
  Serial.begin(115200);
  pinMode(DSMPIN, INPUT);
  pinMode(DSMPIN25, INPUT);
  pinMode(MQ135PIN, INPUT);
  connectWiFi();
  
  dht.setup(DHTPIN, DHTesp::DHT22);
  starttime = millis();
}

void loop() {
  duration = pulseIn(DSMPIN, LOW);
  lowpulseoccupancy += duration;

  duration25 = pulseIn(DSMPIN25, LOW);
  lowpulseoccupancy25 += duration25; 
  
  threshold = digitalRead(MQ135PIN);
 
  if ((millis()-starttime) > sampletime_ms)
  {
    
    float humidity = dht.getHumidity();
    float temp = dht.getTemperature();
    Serial.print("Umidade: ");
    Serial.print(temp);
    String data = "field1=" + String(temp);
    Serial.print(" %\t");
    Serial.print("Temperatura: ");
    Serial.print(humidity);
    data += "&field2=" + String(humidity);
    Serial.println(" *C");
    Serial.print("Low Pulse Occupancy: ");
    Serial.print(lowpulseoccupancy);
    Serial.print("\n");
    
    ratio = lowpulseoccupancy/(sampletime_ms)*10.0;  // Integer percentage 0=>100
    concentration = 0.001915*pow(ratio,2) + 0.09522*ratio - 0.04884; // using spec sheet curve

    ratio25 = lowpulseoccupancy25/(sampletime_ms)*10.0;
    concentration25 = 0.001915*pow(ratio25,2) + 0.09522*ratio25 - 0.04884;

    Serial.println("concentration 2.5 = " + String(concentration25));
    Serial.println("concentration 1.0 = " + String(concentration));
    
    Serial.print("concentration = ");
    Serial.print(concentration - concentration25);
    data += "&field3=" + String(concentration - concentration25);
    data += "&field4=" + String(threshold);
    data += "&latitude=-19.868104";
    data += "&longitude=-43.95538";
    if (temp != 0 && humidity != 0 && concentration != 0)
      data += "&status=1";
    else 
      data += "&status=0";
    Serial.print(" mg/m^3\n");
    Serial.println(data);
    postData(data);
    lowpulseoccupancy = 0;
    lowpulseoccupancy25 = 0;
    starttime = millis();
  }
}
