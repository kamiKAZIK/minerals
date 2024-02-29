
#include <ESP8266WiFi.h>
#include <WiFiManager.h>

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    // some boards need this because of native USB capability
  }

  WiFiManager wifiManager;
  
  IPAddress ip = IPAddress(192, 168, 1, 199);
  IPAddress gateway = IPAddress(192, 168, 1, 1);
  IPAddress subnet = IPAddress(255, 255, 255, 0);
  wifiManager.setSTAStaticIPConfig(ip, gateway, subnet);

  if (!wifiManager.autoConnect("Maskatuoklis 3000")) {
    Serial.println(F("Could not connect to previous SSID!"));
  } else {
    Serial.println(F("Connected!"));
  }
}

void loop(){
  Serial.println("Sleeping!");
  ESP.deepSleep(10e6);
  Serial.println("Awake!");
}
