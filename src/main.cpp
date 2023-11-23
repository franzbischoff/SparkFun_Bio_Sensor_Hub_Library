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

#include <esp_log.h>
#include <esp_system.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <SparkFun_Bio_Sensor_Hub_Library.h>

static const char TAG[] = "main";
// Reset pin, MFIO pin
int resPin = RESPIN;
int mfioPin = MFIOPIN;

// Possible widths: 69, 118, 215, 411us
int width = 411;
// Possible samples: 50, 100, 200, 400, 800, 1000, 1600, 3200 samples/second
// Not every sample amount is possible with every width; check out our hookup
// guide for more information.
int samples = 100;
int pulseWidthVal;
int sampleVal;

TickType_t last_wake_time;

// Takes address, reset pin, and MFIO pin.
SparkFun_Bio_Sensor_Hub bioHub(resPin, mfioPin);

bioData body;
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

#if defined(ARDUINO_ESP32_DEV)
  vTaskDelay((portTICK_PERIOD_MS * 2000)); // delay for printing after reset
  ESP_LOGI(TAG, "ARDUINO_ESP32_DEV");
#endif

  int result = bioHub.begin();
  if (result == 0) { // Zero errors!
    ESP_LOGI(TAG, "Sensor started!");
  }

  ESP_LOGI(TAG, "Configuring Sensor....");
  int error = bioHub.configSensorBpm(MODE_ONE); // Configure Sensor and BPM mode , MODE_TWO also available
  if (error == 0) {                             // Zero errors.
    ESP_LOGI(TAG, "Sensor configured.");
  } else {
    ESP_LOGI(TAG, "Error configuring sensor.");
    ESP_LOGI(TAG, "Error: ");
    ESP_LOGI(TAG, "%d", error);
  }

  // Set pulse width.
  error = bioHub.setPulseWidth(width);
  if (error == 0) { // Zero errors.
    ESP_LOGI(TAG, "Pulse Width Set.");
  } else {
    ESP_LOGI(TAG, "Could not set Pulse Width.");
    ESP_LOGI(TAG, "Error: ");
    ESP_LOGI(TAG, "%d", error);
  }

  // Check that the pulse width was set.
  pulseWidthVal = bioHub.readPulseWidth();
  ESP_LOGI(TAG, "Pulse Width: ");
  ESP_LOGI(TAG, "%d", pulseWidthVal);

  // Set sample rate per second. Remember that not every sample rate is
  // available with every pulse width. Check hookup guide for more information.
  error = bioHub.setSampleRate(samples);
  if (error == 0) { // Zero errors.
    ESP_LOGI(TAG, "Sample Rate Set.");
  } else {
    ESP_LOGI(TAG, "Could not set Sample Rate!");
    ESP_LOGI(TAG, "Error: ");
    ESP_LOGI(TAG, "%d", error);
  }

  // bioHub.set_report_period(100);

  // Check sample rate.
  sampleVal = bioHub.readSampleRate();
  ESP_LOGI(TAG, "Sample rate is set to: ");
  ESP_LOGI(TAG, "%d", sampleVal);

  // Data lags a bit behind the sensor, if you're finger is on the sensor when
  // it's being configured this delay will give some time for the data to catch
  // up.
  //  ESP_LOGI(TAG, "Loading up the buffer with data....");
  //  delay(4000);
  last_wake_time = xTaskGetTickCount();
}

void loop() {

  // Information from the readSensor function will be saved to our "body"
  // variable.

  uint8_t samples = bioHub.numSamplesOutFifo();

  // read all samples in fifo and use most recent one
  while (samples) {
    body = bioHub.readSensorBpm();
    samples--;
  }

  //      ESP_LOGI(TAG, samples);
  //      ESP_LOGI(TAG, ",");
  ESP_LOGI(TAG, "%u", body.heartRate);
  ESP_LOGI(TAG, ",");
  ESP_LOGI(TAG, "%u", body.oxygen);

  samples = bioHub.numSamplesOutFifo();

  // clear fifo before delay
  while (samples) {
    body = bioHub.readSensorBpm();
    samples--;
  }

  vTaskDelayUntil(&last_wake_time, (portTICK_PERIOD_MS * 500));
}

#ifdef __cplusplus
extern "C" {
#endif
/// @brief main function
void app_main(void) {
  setup();

  for (;;) {
    loop();
  }
}

#ifdef __cplusplus
}
#endif
