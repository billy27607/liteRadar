

/*!
 * This is example of using the library in "custom" mode to set presence threshold anf motion threshold levels
 *
 * One will need to appropriately modify the second Serial refernce - here it is Serial 2 -
 * in order to make it work on the board you are working with.
 * 
 * I have found this mode to be much more useful an flexible in tuning the sensor for 
 * a particular area as opposed to the built in scenarios
 * 
 */

#include <Arduino.h>
#include "liteRadar.h"

// #include <HardwareSerial.h>
// HardwareSerial UART(0);

Radar radar = Radar(&Serial2);

void setup() {
    Serial.begin(115200);
    Serial2.begin(115200);

    delay(2000);
    Serial.println("ready");

    byte ret;

    if (!radar.resetRadar()) Serial.println("reset failed");
    delay(1000);

    if(!radar.openCustomMode(MODE_1)) Serial.println("open custom mode failed");
    delay(1000);
    if (!radar.setPresenceThreshold(0x1E)) Serial.println("set presence threshold failed");
    delay(1000);
    if (!radar.setMotionThreshold(0x0E)) Serial.println("set presence threshold failed");
    delay(1000);
    if(!radar.exitCustomMode()) Serial.println("exit custom mode failed");
    delay(1000);

    Serial.println("\nsetup done\n\n\n");
}


void loop() {
    bool changed = radar.updateStatus();
    if (changed) {
        Serial.printf("presence is: %s\n", radar.isPresent() ? "true" : "false");
        Serial.printf("motion value is: %s\n", radar.isMoving() ? "true" : "false");
    }
    delay(20);
}