
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <WiFiManager.h>
#include <DHT.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#define DHTPIN D1
#define DHTTYPE DHT22

#define DEVICE_ID "fb5c282c-2ff9-44a2-b31b-a5fc045d9f30"
#define MQTT_SERVER "test.mosquitto.org"
#define MQTT_PORT 1883
#define MQTT_TOPIC "sensor/readings"
#define T_QUALIFIER 1
#define H_QUALIFIER 2
#define SLEEP_MILLIS 10000

WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(DHTPIN, DHTTYPE);

uint32_t alive = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    // some boards need this because of native USB capability
  }

  WiFiManager wifiManager;
  
  IPAddress ip = IPAddress(192, 168, 178, 199);
  IPAddress gateway = IPAddress(192, 168, 178, 1);
  IPAddress subnet = IPAddress(255, 255, 255, 0);
  IPAddress dns = IPAddress(8, 8, 8, 8);
  wifiManager.setSTAStaticIPConfig(ip, gateway, subnet, dns);

  if (!wifiManager.autoConnect("Maskatuoklis 3000")) {
    Serial.println(F("Could not connect to previous SSID!"));
    delay(3000);
    ESP.restart();
    delay(5000);
  } else {
    Serial.println(F("Connected!"));

    Serial.print(F("Local IP: "));
    Serial.println(WiFi.localIP()); 

    client.setServer(MQTT_SERVER, MQTT_PORT);
    dht.begin();
  }
}

void reconnect() {
  uint8_t tries = 0;

  while (!client.connected() && tries < 3) {
    Serial.print(F("Attempting MQTT connection..."));
    if (client.connect("ESP8266Client")) {
      Serial.println(F("connected"));
    } else {
      Serial.print(F("failed, rc="));
      Serial.print(client.state());
      Serial.println(F(" try again in 3 seconds"));
      delay(3000);

      tries++;
    }
  }
}

void publish(const char* payload) {
  uint8_t tries = 0;

  Serial.print(F("Publishing JSON: "));
  Serial.println(payload);

  while (!client.publish(MQTT_TOPIC, payload, true) && tries < 3) {
    Serial.print(F("failed, rc="));
    Serial.print(client.state());
    Serial.println(F(" try again in 3 seconds"));
    delay(3000);

    tries++;
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }

  if (client.loop()) {
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    float hic = dht.computeHeatIndex(t, h, false);

    Serial.print(F("Humidity: "));
    Serial.print(h);
    Serial.print(F("%, Temperature: "));
    Serial.print(t);
    Serial.print(F("°C, Heat index: "));
    Serial.print(hic);
    Serial.println(F("°C"));

    JsonDocument doc;
    doc["device_id"] = DEVICE_ID;
    doc["readings"][0]["value"] = h;
    doc["readings"][0]["qualifier"] = H_QUALIFIER;
    doc["readings"][1]["value"] = t;
    doc["readings"][1]["qualifier"] = T_QUALIFIER;

    String output;
    serializeJson(doc, output);

    publish(output.c_str());
  }

  ESP.deepSleep(SLEEP_MILLIS * 1000);
}
