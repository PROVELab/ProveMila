#include "../common/pecan.hpp"
#include "Arduino.h"
#include "CAN.h"

void setup() {
    Serial.begin(9600);
    // start the CAN bus at 500 kbps
    if (!CAN.begin(500E3)) {
        Serial.println("Starting CAN failed!");
        while (1)
            ;
    }
    Serial.println("Telemetry startup succesful");
}

void loop() { printf("test"); }
