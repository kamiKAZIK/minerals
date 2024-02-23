#include "RF24.h"
#include "RF24Network.h"
#include "RF24Mesh.h"
#include <SPI.h>

/**
 * Configure the nrf24l01 CE and CS pins
 **/
#define CE_PIN   9
#define CSN_PIN 10

/**
 * User Configuration:
 * nodeID - A unique identifier for each radio. Allows addressing to change dynamically
 * with physical changes to the mesh. (numbers 1-255 allowed)
 **/
#define nodeID 1

RF24 radio(CE_PIN, CSN_PIN);
RF24Network network(radio);
RF24Mesh mesh(radio, network);

uint32_t millisTimer = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    // some boards need this because of native USB capability
  }

  // Set the nodeID manually
  mesh.setNodeID(nodeID);

  // Set the PA Level to MIN and disable LNA for testing & power supply related issues
  radio.begin();
  radio.setPALevel(RF24_PA_MIN, 0);
  radio.setDataRate(RF24_250KBPS);

  // Connect to the mesh
  Serial.println(F("Connecting to the mesh..."));
  if (!mesh.begin()) {
    if (radio.isChipConnected()) {
      do {
        // mesh.renewAddress() will return MESH_DEFAULT_ADDRESS on failure to connect
        Serial.println(F("Could not connect to network.\nConnecting to the mesh..."));
      } while (mesh.renewAddress() == MESH_DEFAULT_ADDRESS);
    } else {
      Serial.println(F("Radio hardware not responding."));
      while (1) {
        // hold in an infinite loop
      }
    }
  }
}


void loop() {
  // Call mesh.update to keep the network updated
  mesh.update();

  // Send to the master node every second
  if (millis() - millisTimer >= 1000) {
    millisTimer = millis();

    // Send an 'M' type to other Node containing the current millis()
    if (!mesh.write(&millisTimer, 'M', sizeof(millisTimer))) {
      if (!mesh.checkConnection()) {
        do {
          Serial.println(F("Reconnecting to mesh network..."));
        } while (mesh.renewAddress() == MESH_DEFAULT_ADDRESS);
      } else {
        Serial.println(F("Send fail, Test OK"));
      }
    } else {
      Serial.print(F("Send OK: "));
      Serial.println(millisTimer);
    }
  }
}
