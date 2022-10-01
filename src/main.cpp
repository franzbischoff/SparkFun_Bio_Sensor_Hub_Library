// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

/*
 This example displays a more manual method of adjusting the way in which the
 MAX30101 gathers data. Specifically we'll look at how to modify the pulse
 length of the LEDs within the MAX30101 which impacts the number of samples
 that can be gathered, so we'll adjust this value as well. In addition we
 gather additional data from the bioData type: LED samples. This data gives
 the number of samples gathered by the MAX30101 for both the red and IR LEDs.
 As a side note you can also choose MODE_ONE and MODE_TWO for configSensorBpm
 as well.
 A summary of the hardware connections are as follows:
 SDA -> SDA
 SCL -> SCL
 RESET -> PIN 4
 MFIO -> PIN 5

 Author: Elias Santistevan
 Date: 8/2019
 SparkFun Electronics

 If you run into an error code check the following table to help diagnose your
 problem:
 1 = Unavailable Command
 2 = Unavailable Function
 3 = Data Format Error
 4 = Input Value Error
 5 = Try Again
 255 = Error Unknown
*/

#include <SparkFun_Bio_Sensor_Hub_Library.hpp>
#include <Wire.h>
#include <TimerOne.h>

bool ready = false;
// Reset pin, MFIO pin
const uint16_t RES_PIN = RESPIN;
const uint16_t MFIO_PIN = MFIOPIN;
// Possible widths: 69, 118, 215, 411us
const uint16_t WIDTH = 411;
// Possible samples: 50, 100, 200, 400, 800, 1000, 1600, 3200 samples/second
// Not every sample amount is possible with every width; check out our hookup
// guide for more information.
const uint16_t SAMPLES = 400;

// Takes address, reset pin, and MFIO pin.
SparkFun_Bio_Sensor_Hub bioHub(RES_PIN, MFIO_PIN);
bioData body;

// following data:
// body.irLed      - Infrared LED counts.
// body.redLed     - Red LED counts.
// body.heartrate  - Heartrate
// body.confidence - Confidence in the heartrate value
// body.oxygen     - Blood oxygen level
// body.status     - Has a finger been sensed?

// short period filter
const float W = 25.0;
const float EPS = 0.05;
const float ALPHA = powf(EPS, 1.0f / W);

// large (wander) period filter
const float WL = 125.0;
const float ALPHAL = powf(EPS, 1.0f / WL);

float redSum = 0.0;
float redNum = 0.0;
float redSum2 = 0.0;
float redNum2 = 0.0;
float irSum = 0.0;
float irNum = 0.0;
float irSum2 = 0.0;
float irNum2 = 0.0;

void control_irq() { ready = true; }

void setup() {

  Serial.begin(115200);
  Wire.begin();
  bioHub.begin();

  bioHub.configSensorBpm(MODE_ONE);
  bioHub.setPulseWidth(WIDTH);   // 18 bits resolution (0-262143)
  bioHub.setSampleRate(SAMPLES); // 18 bits resolution (0-262143)
  Timer1.initialize(4000);       // 250hz
  Timer1.attachInterrupt(control_irq);
  delay(4000); // Wait for sensor to stabilize
}

void loop() {
  body = bioHub.readSensorBpm(); // Read the sensor outside the IRQ, to avoid overload

  if (ready) {
    const float irLed = (float)body.irLed;
    const float redLed = (float)body.redLed;
    int16_t irRes = -3000;
    int16_t redRes = -3000;

    if (irLed > 20000) {
      irSum = irSum * ALPHA + irLed;
      irNum = irNum * ALPHA + 1.0f;
      irSum2 = irSum2 * ALPHAL + irLed;
      irNum2 = irNum2 * ALPHAL + 1.0f;
      irRes = (int16_t)(10 * (irSum / irNum - irSum2 / irNum2));
      if (irRes > 2047 || irRes < -2048) {
        irRes = -3000;
      }
    }

    if (redLed > 20000) {
      redSum = redSum * ALPHA + redLed;
      redNum = redNum * ALPHA + 1.0f;
      redSum2 = redSum2 * ALPHAL + redLed;
      redNum2 = redNum2 * ALPHAL + 1.0f;
      redRes = (int16_t)(20 * (redSum / redNum - redSum2 / redNum2));
      if (redRes > 2047 || redRes < -2048) {
        redRes = -3000;
      }
    }

    Serial.print(irRes);
    Serial.print(",");
    Serial.println(redRes);
    ready = false;
  }

  delay(1); // Just to breath a little
}
