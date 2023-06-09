/*
 * Adafruit Feather M4 CAN Transceiver Example
 */

#include <CANSAME5x.h>
#include <Adafruit_NeoPixel.h>

CANSAME5x CAN;

Adafruit_NeoPixel strip(1, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);

#define MY_PACKET_ID 0xAF

uint32_t timestamp;

void setup() {
  Serial.begin(115200);
  //while (!Serial) delay(10);

  Serial.println("CAN NeoPixel Potentiometer RX/TX demo");

  pinMode(PIN_CAN_STANDBY, OUTPUT);
  digitalWrite(PIN_CAN_STANDBY, false); // turn off STANDBY
  pinMode(PIN_CAN_BOOSTEN, OUTPUT);
  digitalWrite(PIN_CAN_BOOSTEN, true); // turn on booster

  strip.begin();
  strip.setBrightness(50);

  // start the CAN bus at 250 kbps
  if (!CAN.begin(250000)) {
    Serial.println("Starting CAN failed!");
    while (1) delay(10);
  }

  timestamp = millis();
}

void loop() {
  // every 100 ms send out a packet
  if ((millis() - timestamp) > 100) {
    uint16_t pot = analogRead(A5);
    // send a packet with the potentiometer value  
    Serial.print("Sending packet with value ");
    Serial.print(pot);

    CAN.beginPacket(MY_PACKET_ID);
    CAN.write(pot >> 8);
    CAN.write(pot & 0xFF);
    CAN.endPacket();
  
    Serial.println("...sent!");
    timestamp = millis();
  }

  // try to parse any incoming packet
  int packetSize = CAN.parsePacket();

  if (packetSize) {
    // received a packet
    Serial.print("Received ");

    if (CAN.packetExtended()) {
      Serial.print("extended ");
    }

    if (CAN.packetRtr()) {
      // Remote transmission request, packet contains no data
      Serial.print("RTR ");
    }

    Serial.print("packet with id 0x");
    Serial.print(CAN.packetId(), HEX);

    if (CAN.packetRtr()) {
      Serial.print(" and requested length ");
      Serial.println(CAN.packetDlc());
    } else {
      Serial.print(" and length ");
      Serial.println(packetSize);

      uint8_t receivedData[packetSize];
      for (int i=0; i<packetSize; i++) {
        receivedData[i] = CAN.read();
        Serial.print("0x");
        Serial.print(receivedData[i], HEX);
        Serial.print(", ");
      }
      Serial.println();

      uint16_t value = (uint16_t)receivedData[0] << 8 | receivedData[1];
      strip.setPixelColor(0, Wheel(value / 4));
      strip.show();
    }

    Serial.println();
  }
}

uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
