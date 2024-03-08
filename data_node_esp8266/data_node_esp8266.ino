
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <WiFiManager.h>
#include <DHT.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#define DHTPIN D1
#define DHTTYPE DHT22

#define mqtt_server    "test.mosquitto.org"
#define mqtt_port      1883
#define readings_topic "sensor/readings"

WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(DHTPIN, DHTTYPE);

unsigned long lastMsg = 0;

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
    ESP.reset();
    delay(5000);
  } else {
    Serial.println(F("Connected!"));

    Serial.print(F("Local IP: "));
    Serial.println(WiFi.localIP()); 
    
    client.setServer(mqtt_server, mqtt_port);
    dht.begin();
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void loop() {
  Serial.println("Sleeping!");
  ESP.deepSleep(10e6);
  Serial.println("Awake!");

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 10000) {
    lastMsg = now;

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

    unsigned char humidityQualifier = 'H';
    JsonDocument doc;
    doc["sensor"] = "gps";
    doc["readings"][0]["value"] = h;
    doc["readings"][0]["qualifier"] = humidityQualifier;

    String output;
    serializeJson(doc, output);

    client.publish(readings_topic, output.c_str(), true);
  }
}
