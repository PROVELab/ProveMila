#include "../common/pecan.hpp"
#include "Arduino.h"
#include "CAN.h"

PScheduler ps;
void hiHyd(){
    Serial.println("hey hi hyd");
}

void setup() {
    Serial.begin(9600);

    PTask pt;
    pt.function = hiHyd;
    pt.interval = 1000;
    ps.scheduleTask(pt);
}

void loop() {
    ps.mainloop(NULL);
}
