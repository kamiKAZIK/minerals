
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <WiFiManager.h>
#include <DHT.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#define DN_DEBUG true

const uint8_t tQualifier = 1, hQualifier = 2;
const char* deviceId = "fb5c282c-2ff9-44a2-b31b-a5fc045d9f30";

WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(D1, DHT22);
WiFiManager wifiManager;
WiFiManagerParameter mqttDomain("domain", "mqtt domain", "test.mosquitto.org", 64);
WiFiManagerParameter mqttPort("port", "mqtt port", "1883", 5, "pattern='\\d{1,5}'");
WiFiManagerParameter mqttTopic("topic", "mqtt topic", "sensor/readings", 64);
WiFiManagerParameter sleepSeconds("sleep", "sleep (seconds)", "30", 4, "pattern='\\d{1,4}'");

void setup() {
  #ifdef DN_DEBUG
  Serial.begin(115200);
  while (!Serial) {
    // some boards need this because of native USB capability
  }
  #endif

  WiFi.mode(WIFI_STA);

  wifiManager.setConfigPortalBlocking(false);
  wifiManager.setConfigPortalTimeout(120);
  wifiManager.setConnectTimeout(15);

  wifiManager.addParameter(&mqttDomain);
  wifiManager.addParameter(&mqttPort);
  wifiManager.addParameter(&mqttTopic);
  wifiManager.addParameter(&sleepSeconds);

  if (!wifiManager.autoConnect("Maskatuoklis 3000")) {
    #ifdef DN_DEBUG
    Serial.println(F("Could not connect to previous SSID!"));
    #endif
  } else {
    #ifdef DN_DEBUG
    Serial.println(F("Connected!"));
    Serial.print(F("Local IP: "));
    Serial.println(WiFi.localIP()); 
    #endif

    client.setServer(mqttDomain.getValue(), atoi(mqttPort.getValue()));
    dht.begin();
  }
}

void reconnect() {
  uint8_t tries = 0;

  while (!client.connected() && tries < 3) {
    #ifdef DN_DEBUG
    Serial.print(F("Attempting MQTT connection..."));
    #endif

    if (client.connect("ESP8266Client")) {
      #ifdef DN_DEBUG
      Serial.println(F("connected"));
      #endif
    } else {
      #ifdef DN_DEBUG
      Serial.print(F("failed, rc="));
      Serial.print(client.state());
      Serial.println(F(" try again in 2 seconds"));
      #endif

      delay(2000);

      tries++;
    }
  }
}

void publish(const char* payload) {
  uint8_t tries = 0;

  #ifdef DN_DEBUG
  Serial.print(F("Publishing JSON: "));
  Serial.println(payload);
  #endif

  while (!client.publish(mqttTopic.getValue(), payload, true) && tries < 3) {
    #ifdef DN_DEBUG
    Serial.print(F("failed, rc="));
    Serial.print(client.state());
    Serial.println(F(" try again in 2 seconds"));
    #endif

    delay(2000);

    tries++;
  }
}

void loop() {
  if (!wifiManager.process()) {
    if (!client.connected()) {
      reconnect();
    }

    if (client.loop()) {
      float h = dht.readHumidity();
      float t = dht.readTemperature();
      float hic = dht.computeHeatIndex(t, h, false);

      #ifdef DN_DEBUG
      Serial.print(F("Humidity: "));
      Serial.print(h);
      Serial.print(F("%, Temperature: "));
      Serial.print(t);
      Serial.print(F("°C, Heat index: "));
      Serial.print(hic);
      Serial.println(F("°C"));
      #endif

      JsonDocument doc;
      doc["device_id"] = deviceId;
      doc["readings"][0]["value"] = round(h * 10.0) / 10.0;
      doc["readings"][0]["qualifier"] = hQualifier;
      doc["readings"][1]["value"] = round(t * 10.0) / 10.0;
      doc["readings"][1]["qualifier"] = tQualifier;

      String output;
      serializeJson(doc, output);

      publish(output.c_str());
    }

    ESP.deepSleep(atoi(sleepSeconds.getValue()) * 1000000);
  }
}
