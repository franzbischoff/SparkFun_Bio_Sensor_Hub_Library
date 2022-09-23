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

// Reset pin, MFIO pin
const int resPin = RESPIN;
const int mfioPin = MFIOPIN;
// Possible widths: 69, 118, 215, 411us
int width = 69;
// Possible samples: 50, 100, 200, 400, 800, 1000, 1600, 3200 samples/second
// Not every sample amount is possible with every width; check out our hookup
// guide for more information.
int samples = 1600;
int pulseWidthVal;
int sampleVal;

// Takes address, reset pin, and MFIO pin.
SparkFun_Bio_Sensor_Hub bioHub(resPin, mfioPin);

bioData body;

// following data:
// body.irLed      - Infrared LED counts.
// body.redLed     - Red LED counts.
// body.heartrate  - Heartrate
// body.confidence - Confidence in the heartrate value
// body.oxygen     - Blood oxygen level
// body.status     - Has a finger been sensed?

void setup(){

  Serial.begin(115200);

  Wire.begin();
  int result = bioHub.begin();

  int error = bioHub.configSensorBpm(MODE_ONE); // Configure Sensor and BPM mode , MODE_TWO also available

  error = bioHub.setPulseWidth(width);
  error = bioHub.setSampleRate(samples);

  delay(1000);

}

void loop(){

    // Information from the readSensor function will be saved to our "body"
    // variable.
    body = bioHub.readSensorBpm();
    Serial.print(body.irLed);
    Serial.print(",");
    Serial.println(body.redLed);
    // Slow it down or your heart rate will go up trying to keep up
    // with the flow of numbers
}
