#include "sensors.hpp"
#include <Adafruit_Sensor.h>
#include <Adafruit_VL53L0X.h>
#include <Wire.h>
#include "pins.hpp"

/* ---------------------------------------------------- Globals ---------------------------------------------------- */
/* ---------------------------------------------------- Statics ---------------------------------------------------- */
static TwoWire I2CSensors = TwoWire(0);
static Adafruit_VL53L0X lox = Adafruit_VL53L0X();

/* ----------------------------------------- Private Function Declarations ----------------------------------------- */
/* ------------------------------------------ Public Function Definitions ------------------------------------------ */
int sensors::init(void) {
    // Init I2C (default I2C is used by the camera)
    I2CSensors.begin(I2C_SDA, I2C_SCL, (uint32_t) 100000);

    // Init TOF sensor
    if (!lox.begin(0x29, &I2CSensors)) {
        Serial.println("Failed to boot VL53L0X");
        return 1;
    }
    return 0;
}

uint16_t sensors::get_dist_mm(void) {
    VL53L0X_RangingMeasurementData_t measure;
    lox.rangingTest(&measure, false);

    if (measure.RangeStatus != 4) { // 4 is "out of range"
        return measure.RangeMilliMeter;
    }

    return -1;
}

/* ----------------------------------------- Private Function Definitions ------------------------------------------ */
