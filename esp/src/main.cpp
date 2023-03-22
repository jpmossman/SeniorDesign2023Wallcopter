#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_VL53L0X.h>
#include "cam.hpp"
#include "client.hpp"
#include "sensors.hpp"

/* ---------------------------------------------------- Globals ---------------------------------------------------- */
/* --------------------------------------------- Function Declarations --------------------------------------------- */
/* --------------------------------------------- Function Definitions ---------------------------------------------- */
 
void setup() {
    // Start serial
    Serial.begin(9600);

    sensors::init();
    cam::init();
    client::init();
}
 
void loop() {
    // Ensure client connectivity
    // Send distance
    uint16_t dist = sensors::get_dist_mm();
    if (dist != -1) {
        Serial.print("Distance (mm): ");
        Serial.println(dist);
        client::publish("wc/dist",(byte*)(&dist),2);
    }
    else {
        Serial.println("out of range");
    }

    // Send frame buffer
    cam::send_jpg_buffer("wc/frame");
    
    delay(100);
}
