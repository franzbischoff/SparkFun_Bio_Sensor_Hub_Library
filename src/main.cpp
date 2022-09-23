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
const int width = 411;
// Possible samples: 50, 100, 200, 400, 800, 1000, 1600, 3200 samples/second
// Not every sample amount is possible with every width; check out our hookup
// guide for more information.
const int samples = 400;

// Takes address, reset pin, and MFIO pin.
SparkFun_Bio_Sensor_Hub bioHub(resPin, mfioPin);

// bioData body;
// ^^^^^^^^^
// What's this!? This is a type (like "int", "byte", "long") unique to the SparkFun
// Pulse Oximeter and Heart Rate Monitor. Unlike those other types it holds
// specific information on the LED count values of the sensor and ALSO the
// biometric data: heart rate, oxygen levels, and confidence. "bioLedData" is
// actually a specific kind of type, known as a "struct". I chose the name
// "body" but you could use another variable name like "blood", "readings",
// "ledBody" or whatever. Using the variable in the following way gives the
// following data:
// body.irLed      - Infrared LED counts.
// body.redLed     - Red LED counts.
// body.heartrate  - Heartrate
// body.confidence - Confidence in the heartrate value
// body.oxygen     - Blood oxygen level
// body.status     - Has a finger been sensed?

void setup() {

  Serial.begin(115200);

  Wire.begin();
  uint8_t const result = bioHub.begin();
  if (result == 0) { // Zero errors!
    Serial.println("Sensor started!");
  }

  Serial.println("Configuring Sensor....");
  uint8_t error = bioHub.configSensorBpm(MODE_ONE); // Configure Sensor and BPM mode , MODE_TWO also available
  if (error == 0) {                             // Zero errors.
    Serial.println("Sensor configured.");
  } else {
    Serial.println("Error configuring sensor.");
    Serial.print("Error: ");
    Serial.println(error);
  }

  // Set pulse width.
  error = bioHub.setPulseWidth(width);
  if (error == 0) { // Zero errors.
    Serial.println("Pulse Width Set.");
  } else {
    Serial.println("Could not set Pulse Width.");
    Serial.print("Error: ");
    Serial.println(error);
  }

  // Check that the pulse width was set.
  uint16_t const pulseWidthVal = bioHub.readPulseWidth();
  Serial.print("Pulse Width: ");
  Serial.println(pulseWidthVal);

  // Set sample rate per second. Remember that not every sample rate is
  // available with every pulse width. Check hookup guide for more information.
  error = bioHub.setSampleRate(samples);
  if (error == 0) { // Zero errors.
    Serial.println("Sample Rate Set.");
  } else {
    Serial.println("Could not set Sample Rate!");
    Serial.print("Error: ");
    Serial.println(error);
  }

  // Check sample rate.
  uint16_t const sampleVal = bioHub.readSampleRate();
  Serial.print("Sample rate is set to: ");
  Serial.println(sampleVal);

  // Data lags a bit behind the sensor, if you're finger is on the sensor when
  // it's being configured this delay will give some time for the data to catch
  // up.
  Serial.println("Loading up the buffer with data....");
  delay(4000);
}

void loop() {

  // Information from the readSensor function will be saved to our "body"
  // variable.
  bioData const body = bioHub.readSensorBpm();
  Serial.print("Infrared LED counts: ");
  Serial.println(body.irLed);
  Serial.print("Red LED counts: ");
  Serial.println(body.redLed);
  Serial.print("Heartrate: ");
  Serial.println(body.heartRate);
  Serial.print("Confidence: ");
  Serial.println(body.confidence);
  Serial.print("Blood Oxygen: ");
  Serial.println(body.oxygen);
  Serial.print("Status: ");
  Serial.println(body.status);
  // Slow it down or your heart rate will go up trying to keep up
  // with the flow of numbers
  delay(250);
}
