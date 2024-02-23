
#include "RF24Network.h"
#include "RF24.h"
#include "RF24Mesh.h"
#include <SPI.h>

/**
 * Configure the nrf24l01 CE and CS pins
 **/
#define CE_PIN   9
#define CSN_PIN 10

RF24 radio(CE_PIN, CSN_PIN);
RF24Network network(radio);
RF24Mesh mesh(radio, network);

uint32_t millisTimer = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    // some boards need this because of native USB capability
  }

  // Set the nodeID to 0 for the master node
  mesh.setNodeID(0);

  // Set the PA Level to MIN and disable LNA for testing & power supply related issues
  radio.begin();
  radio.setPALevel(RF24_PA_MIN, 0);
  radio.setDataRate(RF24_250KBPS);

  // Connect to the mesh
  if (!mesh.begin()) {
    // if mesh.begin() returns false for a master node, then radio.begin() returned false.
    Serial.println(F("Radio hardware not responding."));
    while (1) {
      // hold in an infinite loop
    }
  }
}


void loop() {
  // Call mesh.update to keep the network updated
  mesh.update();

  // In addition, keep the 'DHCP service' running on the master node so addresses will
  // be assigned to the sensor nodes
  mesh.DHCP();

  // Check for incoming data from the sensors
  if (network.available()) {
    RF24NetworkHeader header;
    network.peek(header);

    uint32_t receivedTimer = 0;
    switch (header.type) {
      // Display the incoming millis() values from the sensor nodes
      case 'M':
        network.read(header, &receivedTimer, sizeof(receivedTimer));
        Serial.print(F("Node ID: "));
        Serial.print(mesh.getNodeID(header.from_node));
        Serial.print(F(" Data: "));
        Serial.println(receivedTimer);
        break;
      default:
        network.read(header, 0, 0);
        Serial.println(header.type);
        break;
    }
  }

  if (millis() - millisTimer > 5000) {
    millisTimer = millis();
    Serial.println(F(" "));
    Serial.println(F("********Assigned Addresses********"));
    for (int i = 0; i < mesh.addrListTop; i++) {
      Serial.print(F("NodeID: "));
      Serial.print(mesh.addrList[i].nodeID);
      Serial.print(F(" RF24Network Address: 0"));
      Serial.println(mesh.addrList[i].address, OCT);
    }
    Serial.println(F("**********************************"));
  }
}
