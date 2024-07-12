#include <WiFiClientSecure.h>
#include <WiFiManager.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#define uS_TO_S_FACTOR 1000000
#define DN_DEBUG true

/*
mosquitto_pub \
  --cert /home/edkazaka2001/Downloads/iot/d817ad829713b29dd3ff7e34b4c2c8c5ddc00dcef50cad4777f8f0ca882dbe2b-certificate.pem.crt \
  --key /home/edkazaka2001/Downloads/iot/d817ad829713b29dd3ff7e34b4c2c8c5ddc00dcef50cad4777f8f0ca882dbe2b-private.pem.key \
  --cafile /home/edkazaka2001/Downloads/iot/AmazonRootCA1.pem \
  -h xxxxxxxxxxxxx-ats.iot.eu-central-1.amazonaws.com \
  -p 8883 \
  -q 0 \
  -d \
  -V mqttv5 \
  -t 'esp32/sensors' \
  -i 'esp32' \
  -m "Hello from Mosquitto"

mosquitto_sub \
  --cert /home/edkazaka2001/Downloads/iot/8724a923afddec3e06fd4eb05e02c5e47b8172b2b19c14cb4d00c88d3a273fee-certificate.pem.crt \
  --key /home/edkazaka2001/Downloads/iot/8724a923afddec3e06fd4eb05e02c5e47b8172b2b19c14cb4d00c88d3a273fee-private.pem.key \
  --cafile /home/edkazaka2001/Downloads/iot/AmazonRootCA1.pem \
  -h xxxxxxxxxxxxx-ats.iot.eu-central-1.amazonaws.com \
  -p 8883 \
  -q 0 \
  -d \
  -t 'esp32/readings' \
  -i 'esp32-sub'
*/

const uint8_t tQualifier = 1, hQualifier = 2;
const char* deviceId = "fb5c282c-2ff9-44a2-b31b-a5fc045d9f30";

static const char AWS_CERT_CA[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
-----END CERTIFICATE-----
)EOF";

static const char AWS_CERT_CRT[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
-----END CERTIFICATE-----
)EOF";

static const char AWS_CERT_PRIVATE[] PROGMEM = R"EOF(
-----BEGIN RSA PRIVATE KEY-----
-----END RSA PRIVATE KEY-----
)EOF";

WiFiClientSecure wifiClient;
PubSubClient mqttClient(wifiClient);

WiFiManager wifiManager;

WiFiManagerParameter mqttDomain("domain", "mqtt domain", "a1j9n17qa1h23g-ats.iot.eu-central-1.amazonaws.com", 128);
WiFiManagerParameter mqttPort("port", "mqtt port", "8883", 5, "pattern='\\d{1,5}'");
WiFiManagerParameter mqttTopic("topic", "mqtt topic", "esp32/readings", 64);
WiFiManagerParameter sleepSeconds("sleep", "sleep (seconds)", "30", 4, "pattern='\\d{1,4}'");

RTC_DATA_ATTR int bootCount = 0;

#ifdef DN_DEBUG
void printWakeupReason() {
  esp_sleep_wakeup_cause_t wakeupReason = esp_sleep_get_wakeup_cause();

  switch(wakeupReason)
  {
    case ESP_SLEEP_WAKEUP_EXT0:
      Serial.println(F("Wakeup caused by external signal using RTC_IO"));
      break;
    case ESP_SLEEP_WAKEUP_EXT1:
      Serial.println(F("Wakeup caused by external signal using RTC_CNTL"));
      break;
    case ESP_SLEEP_WAKEUP_TIMER:
      Serial.println(F("Wakeup caused by timer"));
      break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD:
      Serial.println(F("Wakeup caused by touchpad"));
      break;
    case ESP_SLEEP_WAKEUP_ULP:
      Serial.println(F("Wakeup caused by ULP program"));
      break;
    default:
      Serial.print(F("Wakeup was not caused by deep sleep: "));
      Serial.println(wakeupReason);
      break;
  }
}
#endif

#ifdef DN_DEBUG
void saveParamsCallback () {
  Serial.println(F("Get Params:"));
  
  Serial.print(mqttDomain.getID());
  Serial.print(F(" : "));
  Serial.println(mqttDomain.getValue());
  
  Serial.print(mqttPort.getID());
  Serial.print(F(" : "));
  Serial.println(mqttPort.getValue());

  Serial.print(mqttTopic.getID());
  Serial.print(F(" : "));
  Serial.println(mqttTopic.getValue());

  Serial.print(sleepSeconds.getID());
  Serial.print(F(" : "));
  Serial.println(sleepSeconds.getValue());
}
#endif

void reconnect() {
  uint8_t tries = 0;

  while (!mqttClient.connected() && tries < 3) {
    #ifdef DN_DEBUG
    Serial.print(F("Attempting MQTT connection... "));
    #endif

    if (mqttClient.connect("esp32")) {
      #ifdef DN_DEBUG
      Serial.println(F("Connected!"));
      #endif
    } else {
      #ifdef DN_DEBUG
      Serial.print(F("Failed, rc="));
      Serial.print(mqttClient.state());
      Serial.println(F(" trying again."));
      #endif

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

  while (!mqttClient.publish(mqttTopic.getValue(), payload) && tries < 3) {
    #ifdef DN_DEBUG
    Serial.print(F("Failed, rc="));
    Serial.print(mqttClient.state());
    Serial.println(F(" trying again."));
    #endif

    tries++;
  }
}

void setup() {
  #ifdef DN_DEBUG
  Serial.begin(115200);
  while (!Serial); // some boards need this because of native USB capability
  #endif

  ++bootCount;
  #ifdef DN_DEBUG
  Serial.print(F("Boot number: "));
  Serial.println(String(bootCount));
  printWakeupReason();
  #endif

  WiFi.mode(WIFI_STA);

  wifiClient.setCACert(AWS_CERT_CA);
  wifiClient.setCertificate(AWS_CERT_CRT);
  wifiClient.setPrivateKey(AWS_CERT_PRIVATE);

  wifiManager.addParameter(&mqttDomain);
  wifiManager.addParameter(&mqttPort);
  wifiManager.addParameter(&mqttTopic);
  wifiManager.addParameter(&sleepSeconds);

  wifiManager.setConfigPortalBlocking(false);
  wifiManager.setConfigPortalTimeout(300);
  wifiManager.setConnectTimeout(30);
  #ifdef DN_DEBUG
  wifiManager.setSaveParamsCallback(saveParamsCallback);
  #endif

  if (!wifiManager.autoConnect(deviceId)) {
    #ifdef DN_DEBUG
    Serial.println(F("Could not connect to previous SSID!"));
    #endif
  } else {
    #ifdef DN_DEBUG
    Serial.println(F("Connected!"));
    Serial.print(F("Local IP: "));
    Serial.println(WiFi.localIP()); 
    #endif

    mqttClient.setServer(mqttDomain.getValue(), atoi(mqttPort.getValue()));
//    dht.begin();
  }
}

void loop() {
  if (!wifiManager.process()) {
    if (!mqttClient.connected()) {
      reconnect();
    }

    if (mqttClient.loop()) {
      float h = 50.0;///dht.readHumidity();
      float t = 20.0;//dht.readTemperature();
      float hic = 20.0;//dht.computeHeatIndex(t, h, false);

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

      delay(10000);
    }
  }
}
