/*
  This is an Arduino Library written for the MAXIM 32664 Biometric Sensor Hub 
  The MAX32664 Biometric Sensor Hub is in actuality a small Cortex M4 microcontroller
  with pre-loaded firmware and algorithms used to interact with the a number of MAXIM
  sensors; specifically the MAX30101 Pulse Oximter and Heart Rate Monitor and
  the KX122 Accelerometer. With that in mind, this library is built to
  communicate with a middle-person and so has a unique method of communication
  (family, index, and write bytes) that is more simplistic than writing and reading to 
  registers, but includes a larger set of definable values.  

  SparkFun Electronics
  Date: June, 2019
  Author: Elias Santistevan
kk
  License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

  Feel like supporting our work? Buy a board from SparkFun!
*/


#include "SparkFun_Bio_Sensor_Hub_Library.h"

SparkFun_Bio_Sensor_Hub::SparkFun_Bio_Sensor_Hub(uint8_t address, uint8_t resetPin, uint8_t mfioPin ) { 
  
  _resetPin = resetPin; 
  _mfioPin = mfioPin;
  _address = address; 
  pinMode(_mfioPin, OUTPUT); 
  pinMode(_resetPin, OUTPUT); // Set these pins as output
  
}

// Family Byte: READ_DEVICE_MODE (0x02) Index Byte: 0x00, Write Byte: 0x00
// The following function initializes the sensor. To place the MAX32664 into
// application mode, the MFIO pin must be pulled HIGH while the board is held
// in reset for 10ms. After 50 addtional ms have elapsed the board should be 
// in application mode and will return two bytes, the first 0x00 is a 
// successful communcation byte, followed by 0x00 which is the byte indicating 
// which mode the IC is in. 
uint8_t SparkFun_Bio_Sensor_Hub::begin( TwoWire &wirePort ) {

  _i2cPort = &wirePort;
  //  _i2cPort->begin(); A call to Wire.begin should occur in sketch 
  //  to avoid multiple begins with other sketches.

  digitalWrite(_mfioPin, HIGH); 
  digitalWrite(_resetPin, LOW); 
  delay(10); 
  digitalWrite(_resetPin, HIGH); 
  delay(1000); 
  pinMode(_mfioPin, INPUT_PULLUP); // To be used as an interrupt later

  uint8_t responseByte = readByte(READ_DEVICE_MODE, 0x00, 1); // 0x00 only possible Index Byte.
  return responseByte; 
}

// Family Byte: READ_DEVICE_MODE (0x02) Index Byte: 0x00, Write Byte: 0x00
// The following function puts the MAX32664 into bootloader mode. To place the MAX32664 into
// bootloader mode, the MFIO pin must be pulled LOW while the board is held
// in reset for 10ms. After 50 addtional ms have elapsed the board should be 
// in bootloader mode and will return two bytes, the first 0x00 is a 
// successful communcation byte, followed by 0x08 which is the byte indicating 
// that the board is in bootloader mode. 
uint8_t SparkFun_Bio_Sensor_Hub::beginBootloader( TwoWire &wirePort ) {

  _i2cPort = &wirePort; 
  //  _i2cPort->begin(); A call to Wire.begin should occur in sketch 
  //  to avoid multiple begins with other sketches.
  
  digitalWrite(_mfioPin, LOW); 
  digitalWrite(_resetPin, LOW); 
  delay(10); 
  digitalWrite(_resetPin, HIGH); 
  delay(50);  //Bootloader mode is enabled when this ends.  
  pinMode(_resetPin, OUTPUT); 
  pinMode(_mfioPin, OUTPUT); 
  
  // Let's check to see if the device made it into bootloader mode.  
  uint8_t responseByte = readByte(READ_DEVICE_MODE, 0x00, 1); // 0x00 only possible Index Byte
  return responseByte;

}

// Family Byte: HUB_STATUS (0x00), Index Byte: 0x00, No Write Byte.
// The following function checks the status of the FIFO. 
uint8_t SparkFun_Bio_Sensor_Hub::readSensorHubStatus(){
  
  uint8_t status = readByte(0x00, 0x00, 1); // Just family and index byte. 
  return status; // Will return 0x00

}

// This function sets very basic settings to get sensor and biometric data.
// The biometric data includes data about heartrate, the confidence
// level, SpO2 levels, and whether the sensor has detected a finger or not. 
uint8_t SparkFun_Bio_Sensor_Hub::beginBpm(){

  uint8_t statusChauf = 0;
  
  readRegisterMAX30101(0x07); // Recommended 

  statusChauf = setOutputMode(ALGO_DATA); // Just the data
  if( statusChauf != 1 )
    return statusChauf; 

  statusChauf = setFIFOThreshold(0x01); // One sample before interrupt is fired.
  if( statusChauf != 1 )
    return statusChauf; 

  statusChauf = max30101Control(ENABLE); 
  if( statusChauf != 1 )
    return statusChauf; 

  statusChauf = whrmFastAlgoControl(ENABLE); 
  if( statusChauf != 1 )
    return statusChauf; 
  
  delay(1000);
  return 0; 

}

// This function sets very basic settings to get LED count values from the MAX30101.
// Sensor data includes 24 bit LED values for the three LED channels: Red, IR,
// and Green. 
uint8_t SparkFun_Bio_Sensor_Hub::beginMaxSensor(){

  uint8_t statusChauf; // Our status chauffeur
  readRegisterMAX30101(0x07); // Recommended 

  statusChauf = setOutputMode(SENSOR_DATA); // Just the sensor data (LED)
  if( statusChauf != 1 )
    return statusChauf; 

  statusChauf = setFIFOThreshold(0x01); // One sample before interrupt is fired to the MAX32664
  if( statusChauf != 1 )
    return statusChauf; 

  statusChauf = max30101Control(ENABLE); //Enable Sensor. 
  if( statusChauf != 1 )
    return statusChauf; 

  statusChauf = whrmFastAlgoControl(ENABLE); //Enable algorithm
  if( statusChauf != 1 )
    return statusChauf; 
  
  delay(1000);
  return 0; 

}
// This function sets very basic settings to get sensor and biometric data.
// Sensor data includes 24 bit LED values for the three LED channels: Red, IR,
// and Green. The biometric data includes data about heartrate, the confidence
// level, SpO2 levels, and whether the sensor has detected a finger or not. 
// Of note, the number of samples is set to one. 
uint8_t SparkFun_Bio_Sensor_Hub::beginSensorBpm(){

  uint8_t statusChauf; // Our status chauffeur
  readRegisterMAX30101(0x07); // Recommended 

  statusChauf = setOutputMode(SENSOR_AND_ALGORITHM); // Data and sensor data 
  if( statusChauf != 1 )
    return statusChauf; 

  statusChauf = setFIFOThreshold(0x01); // One sample before interrupt is fired to the MAX32664
  if( statusChauf != 1 )
    return statusChauf; 

  statusChauf = max30101Control(ENABLE); //Enable Sensor. 
  if( statusChauf != 1 )
    return statusChauf; 

  statusChauf = whrmFastAlgoControl(ENABLE); //Enable algorithm
  if( statusChauf != 1 )
    return statusChauf; 
  
  delay(1000);
  return 0; 

}

// This function takes the 8 bytes from the FIFO buffer related to the wrist
// heart rate algortihm: heart rate (uint16_t), confidence (uint8_t) , SpO2 (uint16_t), 
// and the finger detected status (uint8_t). Note that the the algorithm is stated as 
// "wrist" though the sensor only works with the finger. The data is loaded
// into the whrmFifo and returned.  
bioData SparkFun_Bio_Sensor_Hub::readBpm(){

  bioData libBpm; 
  uint8_t statusChauf; // The status chauffeur captures return values. 
  statusChauf = readSensorHubStatus();
  if(statusChauf == 1) // Communication Error
    return; 

  numSamplesOutFIFO(); 

  uint8_t* data =  readFillArray(READ_DATA_OUTPUT, READ_DATA, WHRM_ARRAY_SIZE, bpmArr); 

  // Heart Rate formatting
  libBpm.heartRate = (uint16_t(data[0]) << 8); 
  libBpm.heartRate |= (data[1]); 
  libBpm.heartRate = libBpm.heartRate/10; 

  // Confidence formatting
  libBpm.confidence = data[2]; 

  //Blood oxygen level formatting
  libBpm.oxygen = uint16_t(data[3]) << 8;
  libBpm.oxygen |= data[4]; 
  libBpm.oxygen = libBpm.oxygen/10;

  //"Machine State" - has a finger been detected?
  libBpm.status = data[5];

  return libBpm;

}

// This function takes 9 bytes of LED values from the MAX30101 associated with 
// the RED, IR, and GREEN LEDs. In addition it gets the 8 bytes from the FIFO buffer 
// related to the wrist heart rate algortihm: heart rate (uint16_t), confidence (uint8_t), 
// SpO2 (uint16_t), and the finger detected status (uint8_t). Note that the the algorithm 
// is stated as "wrist" though the sensor only works with the finger. The data is loaded
// into the whrmFifo and returned.  
ledData SparkFun_Bio_Sensor_Hub::readSensor(){ 

  ledData libLedFifo; 
  uint8_t* data =  readFillArray(READ_DATA_OUTPUT, READ_DATA, MAX30101_LED_ARRAY, senArr); 

  // Value of LED one....
  libLedFifo.irLed = long(data[0]) << 16; 
  libLedFifo.irLed |= long(data[1]) << 8; 
  libLedFifo.irLed |= data[2]; 

  // Value of LED two...
  libLedFifo.redLed = long(data[3]) << 16; 
  libLedFifo.redLed |= long(data[4]) << 8; 
  libLedFifo.redLed |= data[5]; 

  // While it's not used in the MAX30101, but it may be used in other sensors,
  // in which case this will still be useful.
  libLedFifo.ledThree = long(data[6]) << 16; 
  libLedFifo.ledThree |= long(data[7]) << 8; 
  libLedFifo.ledThree |= data[8]; 

  // Value of LED three....
  libLedFifo.greenLed = long(data[9]) << 16; 
  libLedFifo.greenLed |= long(data[10]) << 8; 
  libLedFifo.greenLed |= data[11]; 

  return libLedFifo;

}

// This function modifies the pulse width of the MAX30101 LEDs. All of the LEDs
// are modified to the same width. This will affect the number of samples that
// can be collected and will also affect the ADC resolution. 
// Default is 69us - 15 resolution - 50 samples per second.
// Width(us) - Resolution -  Sample Rate
//  69us     -    15      -   <= 3200 (fastest - least resolution)
//  118us    -    16      -   <= 1600
//  215us    -    17      -   <= 1600
//  411us    -    18      -   <= 1000 (slowest - highest resolution)
uint8_t SparkFun_Bio_Sensor_Hub::setPulseWidth(uint8_t width){

  uint8_t bits; 
  uint8_t statusByte;
  uint8_t regVal;
  
  // Make sure the correct pulse width is selected. 
  if( width == 69 )
    bits = 0;
  else if( width == 118 )
    bits = 1;
  else if( width == 215 )
    bits = 2;
  else if( width == 411 )
    bits = 3;
  else
    return INCORR_PARAM;
 
  // Get current register value so that nothing is overwritten.
  regVal = readRegisterMAX30101(CONFIGURATION_REGISTER); 
  regVal &= WIDTH_MASK; // Mask bits to change. 
  regVal |= bits; // Add bits
  statusByte = writeRegisterMAX30101(CONFIGURATION_REGISTER, regVal); // Write Register

  if(statusByte != SUCCESS)
    return statusByte;
  else
    return SUCCESS; 

}

// This function changes the sample rate of the MAX30101 sensor. The sample
// rate is affected by the set pulse width of the MAX30101 LEDs. 
// Default is 69us - 15 resolution - 50 samples per second.
// Width(us) - Resolution -  Sample Rate
//  69us     -    15      -   <= 3200 (fastest - least resolution)
//  118us    -    16      -   <= 1600
//  215us    -    17      -   <= 1600
//  411us    -    18      -   <= 1000 (slowest - highest resolution)
uint8_t SparkFun_Bio_Sensor_Hub::setSampleRate(uint8_t sampRate){

  uint8_t bits; 
  uint8_t statusByte;
  uint8_t regVal; 

  // Make sure the correct sample rate was picked
  if(sampRate == 50)
    bits = 0; 
  else if(sampRate == 100)
    bits = 1; 
  else if(sampRate == 200)
    bits = 2; 
  else if(sampRate == 400)
    bits = 3; 
  else if(sampRate == 800)
    bits = 4; 
  else if(sampRate == 1000)
    bits = 5; 
  else if(sampRate == 1600)
    bits = 6; 
  else if(sampRate == 3200)
    bits = 7; 
  else
    return INCORR_PARAM;

  // Get current register value so that nothing is overwritten.
  regVal = readRegisterMAX30101(CONFIGURATION_REGISTER); 
  regVal &= SAMP_MASK; // Mask bits to change. 
  regVal |= (bits << 2); // Add bits but shift them first to correct position.
  statusByte = writeRegisterMAX30101(CONFIGURATION_REGISTER, regVal); // Write Register

  if(statusByte != SUCCESS)
    return statusByte;
  else
    return SUCCESS; 

}

// Family Byte: SET_DEVICE_MODE (0x01), Index Byte: 0x01, Write Byte: 0x00
// The following function is an alternate way to set the mode of the of
// MAX32664. It can take three parameters: Enter and Exit Bootloader Mode, as
// well as reset. 
// INCOMPLETE
uint8_t SparkFun_Bio_Sensor_Hub::setOperatingMode(uint8_t selection) {
   
    // Must be one of the three....
    if (selection == EXIT_BOOTLOADER || selection == RESET || selection == ENTER_BOOTLOADER)
      { }
    else
      return INCORR_PARAM;

    uint8_t statusByte = writeByte(SET_DEVICE_MODE, 0x00, selection);
    if (statusByte != SUCCESS ) 
      return statusByte; 

    // Here we'll check if the board made it into Bootloader mode...
    uint8_t responseByte = readByte(READ_DEVICE_MODE, 0x00, 1); // 0x00 only possible Index Byte
    return responseByte; // This is in fact the status byte, need second returned byte - bootloader mode

}

// Family Byte: IDENTITY (0x01), Index Byte: READ_MCU_TYPE, Write Byte: NONE
// The following function returns a byte that signifies the microcontoller that
// is in communcation with your host microcontroller. Returns 0x00 for the
// MAX32625 and 0x01 for the MAX32660/MAX32664. 
// INCOMPLETE
uint8_t SparkFun_Bio_Sensor_Hub::getMCUtype() { 

  uint8_t mcu = readByte(IDENTITY, READ_MCU_TYPE, NO_WRITE, 2);  
  return mcu; // Needs the second byte returned, not the first. 

}

// Family Byte: BOOTLOADER_INFO (0x80), Index Byte: BOOTLOADER_VERS (0x00) 
// This function checks the version number of the bootloader on the chip and
// returns a four bytes: Major version Byte, Minor version Byte, Space Byte,
// and the Revision Byte. 
// INCOMPLETE
long SparkFun_Bio_Sensor_Hub::getBootloaderInf() {

  long bootVers = 0;
  uint8_t * revNum = readByte(BOOTLOADER_INFO, BOOTLOADER_VERS, 0x00, 4);   
  if( revNum[1] != SUCCESS )
    return ERR_UNKNOWN; 
  else {
    bootVers |= (revNum[1] << 16);
    bootVers |= (revNum[2] << 8); 
    bootVers |= revNum[3]; 
    return bootVers;
  }

}

// Family Byte: ENABLE_SENSOR (0x44), Index Byte: ENABLE_MAX86140 (0x00), Write
// Byte: enable (parameter - 0x00 or 0x01). 
// This function enables the MAX86140. 
bool SparkFun_Bio_Sensor_Hub::max86140Control(uint8_t senSwitch) {

  if(senSwitch != 0 || senSwitch != 1)
    return false; 

  // Check that communication was successful, not that the sensor is enabled.
  uint8_t statusByte = writeByte(ENABLE_SENSOR, ENABLE_MAX86140, senSwitch);
  if( statusByte == SUCCESS )
    return true; 
  else
    return false; 

}

// Family Byte: ENABLE_SENSOR (0x44), Index Byte: ENABLE_MAX30205 (0x01), Write
// Byte: enable (parameter - 0x00 or 0x01). 
// This function enables the MAX30205. 
bool SparkFun_Bio_Sensor_Hub::max30205Control(uint8_t senSwitch) {

  if(senSwitch != 0 || senSwitch != 1)
    return false; 
  
  // Check that communication was successful, not that the sensor is enabled.
  uint8_t statusByte = writeByte(ENABLE_SENSOR, ENABLE_MAX30205, senSwitch);
  if( statusByte == SUCCESS ) 
    return true; 
  else
    return false; 

}

// Family Byte: ENABLE_SENSOR (0x44), Index Byte: ENABLE_MAX30001 (0x02), Write
// Byte: senSwitch (parameter - 0x00 or 0x01). 
// This function enables the MAX30001. 
bool SparkFun_Bio_Sensor_Hub::max30001Control(uint8_t senSwitch) {

  if(senSwitch != 0 || senSwitch != 1)
    return false; 

  // Check that communication was successful, not that the sensor is enabled.
  uint8_t statusByte = writeByte(ENABLE_SENSOR, ENABLE_MAX30001, senSwitch);
  if( statusByte == SUCCESS ) 
    return true; 
  else
    return false;

}

// Family Byte: ENABLE_SENSOR (0x44), Index Byte: ENABLE_MAX30101 (0x03), Write
// Byte: senSwitch  (parameter - 0x00 or 0x01).
// This function enables the MAX30101. 
bool SparkFun_Bio_Sensor_Hub::max30101Control(uint8_t senSwitch) {

  if(senSwitch == 0 || senSwitch == 1)
    { }
  else  
    return false; 

  // Check that communication was successful, not that the sensor is enabled.
  uint8_t responseByte = writeByte(ENABLE_SENSOR, ENABLE_MAX30101, senSwitch);
  if( responseByte == SUCCESS ) 
    return true; 
  else
    return false; 

}

// Family Byte: ENABLE_SENSOR (0x44), Index Byte: ENABLE_ACCELEROMETER (0x04), Write
// Byte: accepts (parameter - 0x00 or 0x01). 
// This function enables the Accelerometer. 
bool SparkFun_Bio_Sensor_Hub::accelControl(uint8_t accelSwitch) {

  if(accelSwitch != 0 || accelSwitch != 1)
    return false; 
  
  // Check that communication was successful, not that the sensor is enabled.
  uint8_t statusByte = writeByte(ENABLE_SENSOR, ENABLE_ACCELEROMETER, accelSwitch);
  if( statusByte == SUCCESS ) 
    return true; 
  else
    return false; 

}

// Family Byte: OUTPUT_FORMAT (0x10), Index Byte: SET_FORMAT (0x00), 
// Write Byte : outputType (Parameter values in OUTPUT_MODE_WRITE_BYTE)
bool SparkFun_Bio_Sensor_Hub::setOutputMode(uint8_t outputType) {

  if (outputType < PAUSE || outputType > SENSOR_ALGO_COUNTER) // Bytes between 0x00 and 0x07
    return false; 

  // Check that communication was successful, not that the IC is outputting
  // correct format. 
  uint8_t statusByte = writeByte(OUTPUT_FORMAT, SET_FORMAT, outputType);  
  if( statusByte == SUCCESS)
    return true; 
  else
    return false; 

}

// Family Byte: OUTPUT_FORMAT, Index Byte: SET_THRESHOLD, Write byte: intThres
// (parameter - value betwen 0 and 0xFF).
// This function changes the threshold for the FIFO interrupt bit/pin. The
// interrupt pin is the MFIO pin which is set to INPUT after IC initialization
// (begin). 
bool SparkFun_Bio_Sensor_Hub::setFIFOThreshold(uint8_t intThresh) {

  if( intThresh < 0 || intThresh > 255)
    return false; 
 
  // Checks that there was succesful communcation, not that the threshold was
  // set correctly. 
  uint8_t statusByte = writeByte(OUTPUT_FORMAT, SET_THRESHOLD, intThresh); 
  if( statusByte == SUCCESS)
    return true; 
  else
    return false; 

}

// Family Byte: READ_DATA_OUTPUT (0x12), Index Byte: NUM_SAMPLES (0x00), Write
// Byte: NONE
// This function returns the number of samples available in the FIFO. 
uint8_t SparkFun_Bio_Sensor_Hub::numSamplesOutFIFO() {

  uint8_t sampAvail = readByte(READ_DATA_OUTPUT, NUM_SAMPLES, 1); 
  return sampAvail;

}

// Family Byte: READ_DATA_OUTPUT (0x12), Index Byte: READ_DATA (0x00), Write
// Byte: NONE
// This function returns the data in the FIFO. 
uint8_t* SparkFun_Bio_Sensor_Hub::getDataOutFIFO(uint8_t data[]) {

  uint8_t sampAvail = readByte(READ_DATA_OUTPUT, NUM_SAMPLES, NO_WRITE, 1); 
  uint8_t* dataAvail = readFillArray(READ_DATA_OUTPUT, READ_DATA, sampAvail, data); 
  return dataAvail; 

}

// Family Byte: READ_DATA_OUTPUT (0x12), Index Byte: READ_DATA (0x00), Write
// Byte: NONE
// This function adds support for the acceleromter that is NOT included on
// SparkFun's product, The Family Registery of 0x13 and 0x14 is skipped for now. 
uint8_t SparkFun_Bio_Sensor_Hub::numSamplesExternalSensor() {

  uint8_t sampAvail = readByte(READ_DATA_INPUT, SAMPLE_SIZE, ACCELEROMETER, 1); 
  return sampAvail;

}

// Family Byte: WRITE_REGISTER (0x40), Index Byte: WRITE_MAX86140 (0x00), Write Bytes:
// Register Address and Register Value
// This function writes the given register value at the given register address
// for the MAX86140 and MAX86141 Sensor and returns a boolean indicating a successful 
// or non-successful write.  
bool SparkFun_Bio_Sensor_Hub::writeRegisterMAX861X(uint8_t regAddr, uint8_t regVal) {

  uint8_t writeStat = writeByte(WRITE_REGISTER, WRITE_MAX86140, regAddr, regVal);
  if( writeStat == SUCCESS) 
    return true; 
  else
    return false; 
}

// Family Byte: WRITE_REGISTER (0x40), Index Byte: WRITE_MAX30205 (0x01), Write Bytes:
// Register Address and Register Value
// This function writes the given register value at the given register address
// for the MAX30205 sensor and returns a boolean indicating a successful or
// non-successful write. 
bool SparkFun_Bio_Sensor_Hub::writeRegisterMAX30205(uint8_t regAddr, uint8_t regVal) {
 
  uint8_t writeStat = writeByte(WRITE_REGISTER, WRITE_MAX30205, regAddr, regVal);
  if( writeStat == SUCCESS) 
    return true; 
  else
    return false; 
}

// Family Byte: WRITE_REGISTER (0x40), Index Byte: WRITE_MAX30001 (0x02), Write Bytes:
// Register Address and Register Value
// This function writes the given register value at the given register address
// for the MAX30001 sensor and returns a boolean indicating a successful or
// non-successful write. 
bool SparkFun_Bio_Sensor_Hub::writeRegisterMAX30001(uint8_t regAddr, uint8_t regVal) {
  
  uint8_t writeStat = writeByte(WRITE_REGISTER, WRITE_MAX30001, regAddr, regVal);
  if( writeStat == SUCCESS) 
    return true; 
  else
    return false; 
}

// Family Byte: WRITE_REGISTER (0x40), Index Byte: WRITE_MAX30101 (0x03), Write Bytes:
// Register Address and Register Value
// This function writes the given register value at the given register address
// for the MAX30101 sensor and returns a boolean indicating a successful or
// non-successful write. 
bool SparkFun_Bio_Sensor_Hub::writeRegisterMAX30101(uint8_t regAddr, uint8_t regVal) {

  uint8_t writeStat = writeByte(WRITE_REGISTER, WRITE_MAX30101, regAddr, regVal);
  if( writeStat == SUCCESS) 
    return true; 
  else
    return false; 
}

// Family Byte: WRITE_REGISTER (0x40), Index Byte: WRITE_ACCELEROMETER (0x04), Write Bytes:
// Register Address and Register Value
// This function writes the given register value at the given register address
// for the Accelerometer and returns a boolean indicating a successful or
// non-successful write. 
bool SparkFun_Bio_Sensor_Hub::writeRegisterAccel(uint8_t regAddr, uint8_t regVal) {

  uint8_t writeStat = writeByte(WRITE_REGISTER, WRITE_ACCELEROMETER, regAddr, regVal);
  if( writeStat == SUCCESS) 
    return true; 
  else
    return false; 
}

// Family Byte: READ_REGISTER (0x41), Index Byte: READ_MAX86140 (0x00), Write Byte: 
// Register Address
// This function reads the given register address for the MAX86140 and MAX8641
// Sensors and returns the values at that register. 
uint8_t SparkFun_Bio_Sensor_Hub::readRegisterMAX8614X(uint8_t regAddr) {

  uint8_t regCont = readByte(READ_REGISTER, READ_MAX86140, regAddr, 1); 
  return regCont;

}

// Family Byte: READ_REGISTER (0x41), Index Byte: READ_MAX30205 (0x01), Write Byte: 
// Register Address
// This function reads the given register address for the MAX30205 Sensor and
// returns the values at that register. 
uint8_t SparkFun_Bio_Sensor_Hub::readRegisterMAX30205(uint8_t regAddr) {

  uint8_t regCont = readByte(READ_REGISTER, READ_MAX30205, regAddr, 1); 
  return regCont;

}

// Family Byte: READ_REGISTER (0x41), Index Byte: READ_MAX30001 (0x02), Write Byte: 
// Register Address
// This function reads the given register address for the MAX30001 Sensor and
// returns the values at that register. 
uint8_t SparkFun_Bio_Sensor_Hub::readRegisterMAX30001(uint8_t regAddr) {

  uint8_t regCont = readByte(READ_REGISTER, READ_MAX30001, regAddr, 1); 
  return regCont;

}

// Family Byte: READ_REGISTER (0x41), Index Byte: READ_MAX30101 (0x03), Write Byte: 
// Register Address
// This function reads the given register address for the MAX30101 Sensor and
// returns the values at that register. 
uint8_t SparkFun_Bio_Sensor_Hub::readRegisterMAX30101(uint8_t regAddr) {

  uint8_t regCont = readByte(READ_REGISTER, READ_MAX30101, regAddr, 1); 
  return regCont;

}

// Family Byte: READ_REGISTER (0x41), Index Byte: READ_MAX30101 (0x03), Write Byte: 
// Register Address
// This function reads the given register address for the MAX30101 Sensor and
// returns the values at that register. 
uint8_t SparkFun_Bio_Sensor_Hub::readRegisterAccel(uint8_t regAddr) {

  uint8_t regCont = readByte(READ_REGISTER, READ_ACCELEROMETER, regAddr, 1); 
  return regCont;

}

// Family Byte: READ_ATTRIBUTES_AFE (0x42), Index Byte: RETRIEVE_AFE_MAX86140 (0x00)
// This function retrieves the attributes of the AFE (Analog Front End) of the
// MAX8640/1 sensors. It returns the number of bytes in a word for the sensor
// and the number of registers available. 
sensorAttributes SparkFun_Bio_Sensor_Hub::getAFEAttributesMAX86140() {

  sensorAttributes maxAttr; 
  uint8_t tempArray[2]; 
  uint8_t* tempPoint = readFillArray(READ_ATTRIBUTES_AFE, RETRIEVE_AFE_MAX86140, 2, tempArray);
  maxAttr.byteWord = tempPoint[0];
  maxAttr.availRegisters = tempPoint[1];
  delete[] tempArray; 
  return maxAttr;
    
}

// Family Byte: READ_ATTRIBUTES_AFE (0x42), Index Byte: RETRIEVE_AFE_MAX30205 (0x01)
// This function retrieves the attributes of the AFE (Analog Front End) of the
// MAX30205 sensor. It returns the number of bytes in a word for the sensor
// and the number of registers available. 
sensorAttributes SparkFun_Bio_Sensor_Hub::getAFEAttributesMAX30205() {

  sensorAttributes maxAttr; 
  uint8_t tempArray[2]; 
  uint8_t* tempPoint= readFillArray(READ_ATTRIBUTES_AFE, RETRIEVE_AFE_MAX30205, 2, tempArray);
  maxAttr.byteWord = tempPoint[0];
  maxAttr.availRegisters = tempPoint[1];
  delete[] tempArray; 
  return maxAttr;
    
}

// Family Byte: READ_ATTRIBUTES_AFE (0x42), Index Byte: RETRIEVE_AFE_MAX30001 (0x02)
// This function retrieves the attributes of the AFE (Analog Front End) of the
// MAX30001 sensor. It returns the number of bytes in a word for the sensor
// and the number of registers available. 
sensorAttributes SparkFun_Bio_Sensor_Hub::getAFEAttributesMAX30001() {

  sensorAttributes maxAttr; 
  uint8_t tempArray[2]; 
  uint8_t* tempPoint = readFillArray(READ_ATTRIBUTES_AFE, RETRIEVE_AFE_MAX30001, 2, tempArray);
  maxAttr.byteWord = tempPoint[0];
  maxAttr.availRegisters = tempPoint[1];
  delete[] tempArray; 
  return maxAttr;

}

// Family Byte: READ_ATTRIBUTES_AFE (0x42), Index Byte: RETRIEVE_AFE_MAX30101/ (0x03)
// This function retrieves the attributes of the AFE (Analog Front End) of the
// MAX30101 sensor. It returns the number of bytes in a word for the sensor
// and the number of registers available. 
sensorAttributes SparkFun_Bio_Sensor_Hub::getAFEAttributesMAX30101() {
  
  sensorAttributes maxAttr; 
  uint8_t tempArray[2]; 
  uint8_t* tempPoint = readFillArray(READ_ATTRIBUTES_AFE, RETRIEVE_AFE_MAX30101, 2, tempArray);
  maxAttr.byteWord = tempPoint[0];
  maxAttr.availRegisters = tempPoint[1];
  delete[] tempArray; 
  return maxAttr;

    
}


// Family Byte: READ_ATTRIBUTES_AFE (0x42), Index Byte:
// RETRIEVE_AFE_ACCELEROMETER (0x04)
// This function retrieves the attributes of the AFE (Analog Front End) of the
// Accelerometer. It returns the number of bytes in a word for the sensor
// and the number of registers available. 
sensorAttributes SparkFun_Bio_Sensor_Hub::getAFEAttributesAccelerometer() {

  sensorAttributes maxAttr; 
  uint8_t tempArray[2]; 
  uint8_t* tempPoint = readFillArray(READ_ATTRIBUTES_AFE, RETRIEVE_AFE_ACCELEROMETER, 2, tempArray);
  maxAttr.byteWord = tempPoint[0];
  maxAttr.availRegisters = tempPoint[1];
  delete[] tempArray; 
  return maxAttr;
    
}

// Family Byte: DUMP_REGISTERS (0x43), Index Byte: DUMP_REGISTER_MAX86140 (0x00)
// This function returns all registers and register values sequentially of the
// MAX86140/1 Sensors: register zero and register value zero to register n and 
// register value n.
uint8_t* SparkFun_Bio_Sensor_Hub::dumpRegisterMAX86140(uint8_t* regArray, uint8_t totalRegBytes){
  
  uint8_t* regPoint = readByte(DUMP_REGISTERS, DUMP_REGISTER_MAX86140, totalRegBytes, regArray); //Fake read amount
  return regPoint;

}

// Family Byte: DUMP_REGISTERS (0x43), Index Byte: DUMP_REGISTER_MAX30205 (0x01)
// This function returns all registers and register values sequentially of the
// MAX30205 sensor: register zero and register value zero to register n and 
// register value n.
uint8_t* SparkFun_Bio_Sensor_Hub::dumpRegisterMAX30205(uint8_t regArray[8]) {
  
  uint8_t totalRegBytes = 8;
  uint8_t* regPoint = readByte(DUMP_REGISTERS, DUMP_REGISTER_MAX30205, totalRegBytes, regArray); //Fake read amount
  return regPoint;

}

// Family Byte: DUMP_REGISTERS (0x43), Index Byte: DUMP_REGISTER_MAX30001 (0x02)
// This function returns all registers and register values sequentially of the
// MAX30001 sensor: register zero and register value zero to register n and 
// register value n.
uint8_t* SparkFun_Bio_Sensor_Hub::dumpRegisterMAX30001(uint8_t* regArray, uint8_t totalRegBytes) {
  
  uint8_t* regPoint = readFillArray(DUMP_REGISTERS, DUMP_REGISTER_MAX30001, totalRegBytes, regArray); //Fake read amount
  return regPoint;


}

// Family Byte: DUMP_REGISTERS (0x43), Index Byte: DUMP_REGISTER_MAX30101 (0x03)
// This function returns all registers and register values sequentially of the
// MAX30101 sensor: register zero and register value zero to register n and 
// register value n.
uint8_t* SparkFun_Bio_Sensor_Hub::dumpRegisterMAX30101(uint8_t regArray[255]) {
 
  uint8_t totalRegBytes = 255; 
  uint8_t* regPoint  = readFillArray(DUMP_REGISTERS, DUMP_REGISTER_MAX30101, totalRegBytes, regArray); 
  return regPoint;  

}

// Family Byte: DUMP_REGISTERS (0x43), Index Byte: DUMP_REGISTER_ACCELEROMETER (0x04)
// This function returns all registers and register values sequentially of the
// Accelerometer: register zero and register value zero to register n and 
// register value n.
uint8_t* SparkFun_Bio_Sensor_Hub::dumpRegisterAccelerometer(uint8_t* regArray, uint8_t totalRegBytes) {
 
  uint8_t* regPoint = readFillArray(DUMP_REGISTERS, DUMP_REGISTER_ACCELEROMETER, totalRegBytes, regArray); //Fake read amount
  return regPoint; 

}

// Family Byte: CHANGE_ALGORITHM_CONFIG (0x50), Index Byte:
// SET_TARG_PERC (0x00), Write Byte: AGC_GAIN_ID (0x00) 
// This function sets the target percentage of the full-scale ADC range that
// the automatic gain control algorithm uses. It takes a paramater of zero to 
// 100 percent. 
bool SparkFun_Bio_Sensor_Hub::configALGOrange(uint8_t perc) {

  if( perc < 0 || perc > 100)
    return; 

  // Successful communication or no?
  uint8_t statusByte = writeByte(CHANGE_ALGORITHM_CONFIG, SET_TARG_PERC, AGC_GAIN_ID, perc); 
  if( statusByte == SUCCESS )
    return true;
  else
    return false;

}

// Family Byte: CHANGE_ALGORITHM_CONFIG (0x50), Index Byte:
// SET_STEP_SIZE (0x00), Write Byte: AGC_STEP_SIZE_ID (0x01) 
// This function changes the step size toward the target for the AGC algorithm. 
// It takes a paramater of zero to 100 percent. 
bool SparkFun_Bio_Sensor_Hub::configALGOStepSize(uint8_t step) {

  if( step < 0 || step > 100)
    return false; 

  // Successful communication or no?
  uint8_t statusByte = writeByte(CHANGE_ALGORITHM_CONFIG, SET_STEP_SIZE, AGC_STEP_SIZE_ID, step); 
  if( statusByte == SUCCESS )
    return true; 
  else 
    return false; 

}

// Family Byte: CHANGE_ALGORITHM_CONFIG (0x50), Index Byte:
// SET_SENSITIVITY (0x00), Write Byte: AGC_SENSITIVITY_ID (0x02)
// This function changes the sensitivity of the AGC algorithm.
bool SparkFun_Bio_Sensor_Hub::configALGOsensitivity(uint8_t sense) {

  if( sense < 0 || sense > 100 )
    return false; 

  // Successful communication or no?
  uint8_t statusByte = writeByte(CHANGE_ALGORITHM_CONFIG, SET_SENSITIVITY, AGC_SENSITIVITY_ID, sense); 
  if( statusByte == SUCCESS )
    return true; 
  else 
    return false; 

}

// Family Byte: CHANGE_ALGORITHM_CONFIG (0x50), Index Byte:
// SET_AVG_SAMPLES (0x00), Write Byte: AGC_NUM_SAMP_ID (0x03)
// This function changes the number of samples that are averaged. 
// It takes a paramater of zero to 255. 
bool SparkFun_Bio_Sensor_Hub::configALGOsamples(uint8_t avg) {

  if( avg < 0 || avg > 255 )
    return false; 

  // Successful communication or no?
  uint8_t statusByte = writeByte(CHANGE_ALGORITHM_CONFIG, SET_AVG_SAMPLES, AGC_SENSITIVITY_ID, avg); 
  if( statusByte == SUCCESS )
    return true; 
  else 
    return false; 

}

// Family Byte: CHANGE_ALGORITHM_CONFIG (0x50), Index Byte:
// SET_SAMPLE_WHRM (0x02), Write Byte: WHRM_SAMP_RATE_ID (0x00)
// This function sets the sample rate for the wrist heart rate monitor
// (WHRM) algorithm. 
bool SparkFun_Bio_Sensor_Hub::configWHRMsampRate(uint16_t samp) {

  uint8_t statusByte = writeByte(CHANGE_ALGORITHM_CONFIG, SET_SAMPLE_WHRM, WHRM_SAMP_RATE_ID, samp); 
  if( statusByte == SUCCESS)
    return true; 
  else 
    return false; 

}

// Family Byte: CHANGE_ALGORITHM_CONFIG (0x50), Index Byte:
// SET_WHRM_MAX_HEIGHT (0x02), Write Byte: WHRM_MAX_HEIGHT_ID (0x01)
// This function sets the maximum height for the wrist heart rate monitor
// (WHRM) algorithm. 
bool SparkFun_Bio_Sensor_Hub::configWHRMMaxHeight(uint16_t maxHeight) {

  uint8_t statusByte = writeByte(CHANGE_ALGORITHM_CONFIG, SET_WHRM_MAX_HEIGHT, WHRM_MAX_HEIGHT_ID, maxHeight); 
  if( statusByte == SUCCESS)
    return true; 
  else 
    return false; 

}

// Family Byte: CHANGE_ALGORITHM_CONFIG (0x50), Index Byte:
// SET_WHRM_MAX_WEIGHT (0x02), Write Byte: WHRM_MAX_WEIGHT_ID (0x02)
// This function sets the maximum weight for the wrist heart rate monitor
// (WHRM) algorithm. 
bool SparkFun_Bio_Sensor_Hub::configWHRMMaxWeight(uint16_t maxWeight) {

  uint8_t statusByte = writeByte(CHANGE_ALGORITHM_CONFIG, SET_WHRM_MAX_WEIGHT, SET_WHRM_MAX_WEIGHT, maxWeight); 
  if( statusByte == SUCCESS)
    return true; 
  else 
    return false; 

}

// Family Byte: CHANGE_ALGORITHM_CONFIG (0x50), Index Byte:
// SET_WHRM_MAX_AGE (0x02), Write Byte: WHRM_MAX_AGE_ID (0x03)
// This function sets the maximum age for the wrist heart rate monitor
// (WHRM) algorithm. 
bool SparkFun_Bio_Sensor_Hub::configWHRMMaxAge(uint8_t maxAge) {

  uint8_t statusByte = writeByte(CHANGE_ALGORITHM_CONFIG, SET_WHRM_MAX_AGE, WHRM_MAX_AGE_ID, maxAge); 
  if( statusByte == SUCCESS)
    return true; 
  else 
    return false; 

}

// Family Byte: CHANGE_ALGORITHM_CONFIG (0x50), Index Byte:
// SET_WHRM_MIN_HEIGHT (0x02), Write Byte: WHRM_MIN_HEIGHT_ID (0x04)
// This function sets the minimum height for the wrist heart rate monitor
// (WHRM) algorithm. 
bool SparkFun_Bio_Sensor_Hub::configWHRMMinHeight(uint16_t minHeight) {

  uint8_t statusByte = writeByte(CHANGE_ALGORITHM_CONFIG, SET_WHRM_MIN_HEIGHT, WHRM_MIN_HEIGHT_ID, minHeight); 
  if( statusByte == SUCCESS)
    return true; 
  else 
    return false; 

}

// Family Byte: CHANGE_ALGORITHM_CONFIG (0x50), Index Byte:
// SET_WHRM_MIN_WEIGHT (0x02), Write Byte: WHRM_MIN_WEIGHT_ID (0x05)
// This function sets the minimum weight for the wrist heart rate monitor
// (WHRM) algorithm. 
bool SparkFun_Bio_Sensor_Hub::configWHRMMinWeight(uint16_t minWeight) {

  uint8_t statusByte = writeByte(CHANGE_ALGORITHM_CONFIG, SET_WHRM_MIN_WEIGHT, WHRM_MIN_WEIGHT_ID, minWeight); 
  if( statusByte == SUCCESS)
    return true; 
  else 
    return false; 

}

// Family Byte: CHANGE_ALGORITHM_CONFIG (0x50), Index Byte:
// SET_WHRM_MIN_AGE (0x02), Write Byte: WHRM_MIN_AGE_ID (0x06)
// This function sets the minimum age for the wrist heart rate monitor
// (WHRM) algorithm. 
bool SparkFun_Bio_Sensor_Hub::configWHRMMinAge(uint8_t minAge) {

  uint8_t statusByte = writeByte(CHANGE_ALGORITHM_CONFIG, SET_WHRM_MIN_AGE, WHRM_MIN_AGE_ID, minAge); 
  if( statusByte == SUCCESS)
    return true; 
  else 
    return false; 

}

// Family Byte: CHANGE_ALGORITHM_CONFIG (0x50), Index Byte:
// SET_WHRM_DEFAULT_HEIGHT (0x02), Write Byte: WHRM_DEF_HEIGHT_ID (0x07)
// This function sets the default height for the wrist heart rate monitor
// (WHRM) algorithm. 
bool SparkFun_Bio_Sensor_Hub::configWHRMDefHeight(uint16_t defHeight) {

  uint8_t statusByte = writeByte(CHANGE_ALGORITHM_CONFIG, SET_WHRM_DEFAULT_HEIGHT, WHRM_DEF_HEIGHT_ID, defHeight); 
  if( statusByte == SUCCESS)
    return true; 
  else 
    return false; 

}

// Family Byte: CHANGE_ALGORITHM_CONFIG (0x50), Index Byte:
// SET_WHRM_DEFAULT_WEIGHT (0x02), Write Byte: WHRM_DEF_WEIGHT_ID (0x08)
// This function sets the default weight for the wrist heart rate monitor
// (WHRM) algorithm. 
bool SparkFun_Bio_Sensor_Hub::configWHRMDefWeight(uint16_t defWeight) {

  uint8_t statusByte = writeByte(CHANGE_ALGORITHM_CONFIG, SET_WHRM_DEFAULT_WEIGHT, WHRM_DEF_WEIGHT_ID, defWeight); 
  if( statusByte == SUCCESS)
    return true; 
  else 
    return false; 

}

// Family Byte: CHANGE_ALGORITHM_CONFIG (0x50), Index Byte:
// SET_WHRM_DEFAULT_AGE (0x02), Write Byte: WHRM_DEF_AGE_ID (0x09)
// This function sets the default age for the wrist heart rate monitor
// (WHRM) algorithm. 
bool SparkFun_Bio_Sensor_Hub::configWHRMDefAge(uint8_t defAge) {

  uint8_t statusByte = writeByte(CHANGE_ALGORITHM_CONFIG, SET_WHRM_DEFAULT_AGE, WHRM_DEF_AGE_ID, defAge); 
  if( statusByte == SUCCESS)
    return true; 
  else 
    return false; 

}

// Family Byte: CHANGE_ALGORITHM_CONFIG (0x50), Index Byte:
// SET_WHRM_BPM (0x02), Write Byte: WHRM_BPM_INIT (0x0A)
// This function sets the maximum age for the wrist heart rate monitor
// (WHRM) algorithm. 
bool SparkFun_Bio_Sensor_Hub::configWHRMBPM(uint8_t bpm) {

  uint8_t statusByte = writeByte(CHANGE_ALGORITHM_CONFIG, SET_WHRM_BPM, WHRM_BPM_INIT, bpm); 
  if( statusByte == SUCCESS)
    return true; 
  else 
    return false; 

}

// Family Byte: CHANGE_ALGORITHM_CONFIG (0x50), Index Byte:
// SET_PULSE_OX_COEF (0x02), Write Byte: MAXIMFAST_COEF_ID (0x0B)
// This function takes three values that are used as the Sp02 coefficients.
// These three values are multiplied by 100,000; default values are in order: 159584, -3465966, and 11268987.   
bool SparkFun_Bio_Sensor_Hub::configWHRMCoef(long coef1, long coef2, long coef3) {

  long coefArr[3] = {coef1, coef2, coef3};

  uint8_t statusByte = writeLongBytes(CHANGE_ALGORITHM_CONFIG, SET_PULSE_OX_COEF, MAXIMFAST_COEF_ID, coefArr); 
  delete[] coefArr;
  if( statusByte == SUCCESS)
    return true; 
  else 
    return false; 

}

// Family Byte: CHANGE_ALGORITHM_CONFIG (0x50), Index Byte: SET_EXPOSURE_CNTRL
// (0x02), Write Byte: WHRM_AEC_ID (0x0B)
// This function enables or disables automatic exposure control (AEC). The
// function takes the parameter zero for disable and one for enable. 
bool SparkFun_Bio_Sensor_Hub::autoExpCont(uint8_t enable) {
  
  if( enable != 0 || enable != 1)
    return false; 

  uint8_t statusByte = writeByte( CHANGE_ALGORITHM_CONFIG, SET_EXPOSURE_CNTRL, WHRM_AEC_ID, enable);

  if( statusByte == SUCCESS)
    return true; 
  else 
    return false; 

}

// Family Byte: CHANGE_ALGORITHM_CONFIG (0x50), Index Byte: 
// SET_SKIN_CONTACT_DET (0x02), Write Byte: WHRM_SCD_ID (0x0C)
// This function enables or disables skin contact detection. The
// function takes the parameter zero for disable and one for enable. 
bool SparkFun_Bio_Sensor_Hub::skinDetectControl(uint8_t enable) {
  
  if( enable != 0 || enable != 1)
    return false; 

  uint8_t statusByte = writeByte( CHANGE_ALGORITHM_CONFIG, SET_SKIN_CONTACT_DET, WHRM_SCD_ID, enable);

  if( statusByte == SUCCESS)
    return true; 
  else 
    return false; 

}

// Family Byte: CHANGE_ALGORITHM_CONFIG (0x50), Index Byte: 
// SET_PHOTO_DETECT (0x02), Write Byte: WHRM_PD_ID (0x0D)
// This function sets target photo detector current period in seconds.
bool SparkFun_Bio_Sensor_Hub::adjustPhotoDet(uint16_t per) {
  
  uint8_t statusByte = writeByte( CHANGE_ALGORITHM_CONFIG, SET_PHOTO_DETECT, WHRM_PD_ID, per);

  if( statusByte == SUCCESS)
    return true; 
  else 
    return false; 

}

// Family Byte: CHANGE_ALGORITHM_CONFIG (0x50), Index Byte: 
// SET_SCD_DEBOUNCE (0x02), Write Byte: WHRM_SCD_DEBOUNCE_ID (0x0E)
// This function sets the skin contract detect debounce window. It's not clear
// if this is in seconds or not in the datasheet.
bool SparkFun_Bio_Sensor_Hub::setSCDWindow(uint16_t time) {
  
  uint8_t statusByte = writeByte( CHANGE_ALGORITHM_CONFIG, SET_SCD_DEBOUNCE, WHRM_SCD_DEBOUNCE_ID, time);

  if( statusByte == SUCCESS)
    return true; 
  else 
    return false; 

}

// Family Byte: CHANGE_ALGORITHM_CONFIG (0x50), Index Byte: 
// SET_WHRM_THRESH (0x02), Write Byte: WHRM_MOTION_ID (0x0F)
// This function sets motion magnitude threshold in 0.1g
bool SparkFun_Bio_Sensor_Hub::setMotionMag(uint16_t mag) {
  
  uint8_t statusByte = writeByte( CHANGE_ALGORITHM_CONFIG, SET_WHRM_THRESH, WHRM_MOTION_ID, mag);

  if( statusByte == SUCCESS)
    return true; 
  else 
    return false; 

}

// Family Byte: CHANGE_ALGORITHM_CONFIG (0x50), Index Byte: 
// SET_WHRM_MIN_PD (0x02), Write Byte: WHRM_MIN_PD_ID (0x10)
// This function changes the minimum photodetector currrent in 0.1mA
// increments. 
bool SparkFun_Bio_Sensor_Hub::changePDCurrent(uint16_t curr) {
  
  uint8_t statusByte = writeByte( CHANGE_ALGORITHM_CONFIG, SET_WHRM_MIN_PD, WHRM_MIN_PD_ID, curr);

  if( statusByte == SUCCESS)
    return true; 
  else 
    return false; 

}

// Family Byte: CHANGE_ALGORITHM_CONFIG (0x50), Index Byte: 
// SET_WHRM_PPG (0x02), Write Byte: WHRM_PPG_PD_ID (0x11)
// This function changes the source of the photoplethysmography (PPG) signal for 
// the photodetector (PD). The paramater "pd" accepts one of three values: zero - PD1, 
// one - PD2, and three - PD1 and PD2.
bool SparkFun_Bio_Sensor_Hub::changePPGSource(uint8_t pd) {

  if( pd < 0 || pd > 3)
    return false; 
  
  uint8_t statusByte = writeByte( CHANGE_ALGORITHM_CONFIG, SET_WHRM_PPG, WHRM_PPG_PD_ID, pd );

  if( statusByte == SUCCESS)
    return true; 
  else 
    return false; 

}

// Family Byte: CHANGE_ALGORITHM_CONFIG (0x50), Index Byte: 
// SET_BPT_MED (0x04), Write Byte: BPT_BLOOD_PRESSURE_ID (0x00)
// The function configure the blood pressure trending (BPT) algorithm for
// the users that are on blood pressure medicine. The parameter accepts the
// value of zero (not using) or one (using). 
bool SparkFun_Bio_Sensor_Hub::bptMedicine(uint8_t onbpm) {

  if( onbpm != 0 || onbpm != 1)
    return false; 
  
  uint8_t statusByte = writeByte( CHANGE_ALGORITHM_CONFIG, SET_BPT_MED, BPT_BLOOD_PRESSURE_ID, onbpm );

  if( statusByte == SUCCESS)
    return true; 
  else 
    return false; 

}

// Family Byte: CHANGE_ALGORITHM_CONFIG (0x50), Index Byte: 
// SET_BPT_DIASTOLIC (0x04), Write Byte: BPT_DIASTOLIC_ID (0x01)
// This funciton writes the three givin diastolic BP byte values needed by the
// calibration procedure.  
bool SparkFun_Bio_Sensor_Hub::setDiastolicVal(uint8_t val1, uint8_t val2, uint8_t val3) {
  
  _i2cPort->beginTransmission(_address);     
  _i2cPort->write(CHANGE_ALGORITHM_CONFIG);    
  _i2cPort->write(SET_BPT_DIASTOLIC);    
  _i2cPort->write(BPT_DIASTOLIC_ID); 
  _i2cPort->write(val1);     
  _i2cPort->write(val2);     
  _i2cPort->write(val3);     
  _i2cPort->endTransmission(); 
  delay(CMD_DELAY); 

  _i2cPort->requestFrom(_address, 1); // Status Byte, success or no? 0x00 is a successful transmit
  uint8_t statusByte = _i2cPort->read(); 
  if( statusByte == SUCCESS ) 
    return true; 
  else 
    return false; 

}

// Family Byte: CHANGE_ALGORITHM_CONFIG (0x50), Index Byte: 
// SET_BPT_SYSTOLIC (0x04), Write Byte: BPT_SYSTOLIC_ID (0x02)
// This funciton writes the three givin systolic BP byte values needed by the
// calibration procedure.  
bool SparkFun_Bio_Sensor_Hub::setSystolicVal(uint8_t val1, uint8_t val2, uint8_t val3) {
  
  _i2cPort->beginTransmission(_address);     
  _i2cPort->write(CHANGE_ALGORITHM_CONFIG);    
  _i2cPort->write(SET_BPT_SYSTOLIC);    
  _i2cPort->write(BPT_SYSTOLIC_ID); 
  _i2cPort->write(val1);     
  _i2cPort->write(val2);     
  _i2cPort->write(val3);     
  _i2cPort->endTransmission(); 
  delay(CMD_DELAY); 

  _i2cPort->requestFrom(_address, 1); // Status Byte, success or no? 0x00 is a successful transmit
  uint8_t statusByte = _i2cPort->read(); 
  if( statusByte == SUCCESS ) 
    return true; 
  else 
    return false; 

}

// Family Byte: CHANGE_ALGORITHM_CONFIG (0x50), Index Byte: SET_BPT_EST_DATE
// (0x04), Write Byte: BPT_DATE_ID (0x04)
// This function sets the estimation date with the given month/day integer. 
bool SparkFun_Bio_Sensor_Hub::setBPTEstimationDate(uint16_t monthDay) {

  uint16_t statusByte = writeByte(CHANGE_ALGORITHM_CONFIG, SET_BPT_EST_DATE, BPT_DATE_ID, monthDay);
  if (statusByte == SUCCESS)
    return true;
  else
    return false;

}

// Family Byte: CHANGE_ALGORITHM_CONFIG (0x50), Index Byte: SET_BPT_REST
// (0x04), Write Byte: BPT_RESTING_ID (0x05)
// This function adjusts the blood pressure trending algorithm for a user that
// is resting (zero) or not resting (one). 
bool SparkFun_Bio_Sensor_Hub::setUserResting(uint8_t resting) {

  if( resting != 0 || resting != 1)
    return false; 

  uint16_t statusByte = writeByte(CHANGE_ALGORITHM_CONFIG, SET_BPT_REST, BPT_RESTING_ID, resting);
  if (statusByte == SUCCESS)
    return true;
  else
    return false;

}

// Family Byte: CHANGE_ALGORITHM_CONFIG (0x50), Index Byte: SET_BPT_SPO2_COEF
// (0x04), Write Byte: BPT_SP02_COEF_ID (0x06)
// This function sets the given Sp02 coefficients for the blood pressure trending
// algorithm. 
bool SparkFun_Bio_Sensor_Hub::adjustBPTcoef(long spCoef1, long spCoef2, long spCoef3 ) {
  
  long coefArr[3] = { spCoef1, spCoef2, spCoef3 };

  uint16_t statusByte = writeLongBytes(CHANGE_ALGORITHM_CONFIG, SET_BPT_SPO2_COEF, BPT_SP02_COEF_ID, coefArr);
  delete[] coefArr;
  if (statusByte == SUCCESS)
    return true;
  else
    return false;

}

// Family Byte: CHANGE_ALGORITHM_CONFIG (0x50), Index Byte: SET_WSPO2_COEF
// (0x05), Write Byte: WSP02_COEF_ID (0x00)
// This function sets the given wrist Sp02 (WSp02) coefficients for WSp02
// algorithm. Defaults are in order: 159584, -3465966, and 11268987. 
bool SparkFun_Bio_Sensor_Hub::adjustWSP02Coef(long wspCoef1, long wspCoef2, long wspCoef3 ) {
  
  long coefArr[3] = { wspCoef1, wspCoef2, wspCoef3 };

  uint16_t statusByte = writeLongBytes(CHANGE_ALGORITHM_CONFIG, SET_WSPO2_COEF, WSP02_COEF_ID, coefArr);
  delete[] coefArr;
  if (statusByte == SUCCESS)
    return true;
  else
    return false;

}


// Family Byte: CHANGE_ALGORITHM_CONFIG (0x50), Index Byte: SET_WSP02_SRATE
// (0x05), Write Byte: WSP02_SAMPLE_RATE_ID (0x01)
// This function changes the wrist Sp02 sample rate to 100Hz (zero) or 25Hz
// (one).
bool SparkFun_Bio_Sensor_Hub::changeWSP02SampRate(uint8_t rate) {

  if( rate != 0 || rate != 1)
    return false; 
  
  uint16_t statusByte = writeByte(CHANGE_ALGORITHM_CONFIG, SET_WSP02_SRATE, WSP02_SAMPLE_RATE_ID, rate);
  if (statusByte == SUCCESS)
    return true;
  else
    return false;

}

// Family Byte: CHANGE_ALGORITHM_CONFIG (0x50), Index Byte: SET_WSP02_RUN
// (0x05), Write Byte: WSP02_RUN_MODE_ID (0x02)
// This function changes the writs Sp02 algorithm run mode from continuous
// (zero), from/to one-shot (one).
bool SparkFun_Bio_Sensor_Hub::changeWSP02RunMode(uint8_t mode) {

  if( mode != 0 || mode != 1)
    return false; 
  
  uint16_t statusByte = writeByte(CHANGE_ALGORITHM_CONFIG, SET_WSP02_RUN, WSP02_RUN_MODE_ID, mode);
  if (statusByte == SUCCESS)
    return true;
  else
    return false;

}

// Family Byte: CHANGE_ALGORITHM_CONFIG (0x50), Index Byte: SET_WSP02_AGC
// (0x05), Write Byte: WSP02_AGC_MODE_ID (0x03)
// This function changes the wrist Sp02 algorithm's AGC mode. You can disable
// it (zero) or enable it (one). 
bool SparkFun_Bio_Sensor_Hub::changeWSP02AGCMode(uint8_t enable) {

  if( enable != 0 || enable != 1)
    return false; 
  
  uint16_t statusByte = writeByte(CHANGE_ALGORITHM_CONFIG, SET_WSP02_AGC, WSP02_AGC_MODE_ID, enable);
  if (statusByte == SUCCESS)
    return true;
  else
    return false;

}

// Family Byte: CHANGE_ALGORITHM_CONFIG (0x50), Index Byte:
// SET_WSP02_MOT_DETECT (0x05), Write Byte: WSP02_MOT_DTCT_ID (0x04)
// This function enables (one) or disables (zero) motion detect.
bool SparkFun_Bio_Sensor_Hub::enableWSP02MotDet(uint8_t enable) {

  if( enable != 0 || enable != 1)
    return false; 
  
  uint16_t statusByte = writeByte(CHANGE_ALGORITHM_CONFIG, SET_WSP02_MOT_DETECT, WSP02_MOT_DTCT_ID, enable);
  if (statusByte == SUCCESS)
    return true;
  else
    return false;

}

// Family Byte: CHANGE_ALGORITHM_CONFIG (0x50), Index Byte:
// SET_WSP02_DTCT_PER (0x05), Write Byte: WSP02_MOT_DTCT_PER_ID (0x05)
// This function changes the period of the motion detect and though the
// datasheet does not specify, I assume is in seconds. 
bool SparkFun_Bio_Sensor_Hub::enableWSP02MotDetPer(uint16_t detPer) {

  uint16_t statusByte = writeByte(CHANGE_ALGORITHM_CONFIG, SET_WSP02_DTCT_PER, WSP02_MOT_DTCT_PER_ID, detPer);
  if (statusByte == SUCCESS)
    return true;
  else
    return false;

}

// Family Byte: CHANGE_ALGORITHM_CONFIG (0x50), Index Byte:
// SET_WSP02_THRESH (0x05), Write Byte: WSP02_MOT_THRESH_ID (0x06)
// This function changes the motion threshold for the WSp02 algorithm. The
// given number is multiplied by 100,000. 
bool SparkFun_Bio_Sensor_Hub::setWSP02MotThresh(long threshVal) {

  _i2cPort->beginTransmission(_address);     
  _i2cPort->write(CHANGE_ALGORITHM_CONFIG);    
  _i2cPort->write(SET_WSP02_THRESH);    
  _i2cPort->write(WSP02_MOT_THRESH_ID); 
  _i2cPort->write(threshVal >> 24); 
  _i2cPort->write(threshVal >> 16); 
  _i2cPort->write(threshVal >> 8); 
  _i2cPort->write(threshVal); 
  _i2cPort->endTransmission(); 
  delay(CMD_DELAY); 

  _i2cPort->requestFrom(_address, 1); // Status Byte, success or no? 0x00 is a successful transmit
  uint8_t statusByte = _i2cPort->read(); 
  if( statusByte == SUCCESS )
    return true; 
  else
    return false; 

}

// Family Byte: CHANGE_ALGORITHM_CONFIG (0x50), Index Byte: SET_WSP02_AGC_TOUT
// (0x05), Write Byte: WSP02_AGC_TO_ID (0x07)
// This function changes the timeout period of the wrist Sp02 AGC algorithm. The
// paramter should be given in seconds. 
bool SparkFun_Bio_Sensor_Hub::setWSP02AGCTimeout(uint8_t toVal) {

  uint8_t statusByte = writeByte( CHANGE_ALGORITHM_CONFIG, SET_WSP02_AGC_TOUT, WSP02_AGC_TO_ID, toVal );
  if( statusByte == SUCCESS )
    return true; 
  else 
    return false; 
}


// Family Byte: CHANGE_ALGORITHM_CONFIG (0x50), Index Byte: SET_WSP02_ALG_TOUT
// (0x05), Write Byte: WSP02_ALGO_TO_ID (0x08)
// This function changes the timeout period of the wrist Sp02 algorithm. The
// paramter should be given in seconds. 
bool SparkFun_Bio_Sensor_Hub::setWSP02ALGOTimeout(uint8_t toVal) {

  uint8_t statusByte = writeByte( CHANGE_ALGORITHM_CONFIG, SET_WSP02_ALG_TOUT, WSP02_ALGO_TO_ID, toVal );
  if( statusByte == SUCCESS )
    return true; 
  else 
    return false; 
}

// Family Byte: CHANGE_ALGORITHM_CONFIG (0x50), Index Byte: SET_WSP02_PPG_SIG
// (0x05), Write Byte: WSP02_PD_CONFIG (0x09)
// This function changes the source of the photoplethysmographic source for the wrist Sp02 algorithm.
// The parameter choses the photodetector to use: PD1 (0x01) or PD2 (0x02). 
bool SparkFun_Bio_Sensor_Hub::setWSP02PPGSource(uint8_t pd) {
  
  if( pd != 1 || pd != 2 )
    return false; 

  uint8_t statusByte = writeByte( CHANGE_ALGORITHM_CONFIG, SET_WSP02_PPG_SIG, WSP02_PD_CONFIG, pd );
  if( statusByte == SUCCESS )
    return true; 
  else 
    return false; 

}

// Family Byte: READ_ALGORITHM_CONFIG (0x51), Index Byte:
// READ_AGC_PERCENTAGE (0x00), Write Byte: READ_AGC_PERC_ID (0x00) 
// This function reads and returns the currently set target percentage 
// of the full-scale ADC range that the Automatic Gain Control algorithm is using. 
uint8_t SparkFun_Bio_Sensor_Hub::readALGOrange() {

  uint8_t range = readByte(READ_ALGORITHM_CONFIG, READ_AGC_PERCENTAGE, READ_AGC_PERC_ID, 2); 
  return range; 

}

// Family Byte: READ_ALGORITHM_CONFIG (0x51), Index Byte:
// READ_AGC_STEP_SIZE (0x00), Write Byte: READ_AGC_STEP_SIZE_ID (0x01) 
// This function returns the step size toward the target for the AGC algorithm. 
// It returns a value between zero and 100 percent. 
uint8_t SparkFun_Bio_Sensor_Hub::readALGOStepSize() {

  uint8_t stepSize = readByte(READ_ALGORITHM_CONFIG, READ_AGC_STEP_SIZE, READ_AGC_STEP_SIZE_ID, 2); 
  return stepSize;
}

// Family Byte: READ_ALGORITHM_CONFIG (0x51), Index Byte:
// READ_AGC_SENSITIVITY_ID (0x00), Write Byte: READ_AGC_SENSITIVITY_ID (0x02)
// This function returns the sensitivity (percentage) of the automatic gain control. 
uint8_t SparkFun_Bio_Sensor_Hub::readALGOsensitivity() {

  uint8_t sensitivity = readByte(READ_ALGORITHM_CONFIG, READ_AGC_SENSITIVITY, READ_AGC_SENSITIVITY_ID, 2); 
  return sensitivity; 

}

// Family Byte: READ_ALGORITHM_CONFIG (0x51), Index Byte:
// READ_AGC_NUM_SAMPLES (0x00), Write Byte: READ_AGC_NUM_SAMPLES_ID (0x03)
// This function changes the number of samples that are averaged. 
// It takes a paramater of zero to 255. 
uint8_t SparkFun_Bio_Sensor_Hub::readALGOsamples() {

  uint8_t samples = readByte(READ_ALGORITHM_CONFIG, READ_AGC_NUM_SAMPLES, READ_AGC_NUM_SAMPLES_ID, 2); 
  return samples; 

}

// Family Byte: READ_ALGORITHM_CONFIG (0x51), Index Byte:
// READ_WHRM_SAMPLE_RATE (0x02), Write Byte: READ_WHRM_SAMPLE_RATE_ID (0x00)
// This function reads the sample rate for the wrist heart rate monitor
// (WHRM) algorithm. 
uint8_t SparkFun_Bio_Sensor_Hub::readWHRMsampRate() {

  uint8_t sampRate = readByte(READ_ALGORITHM_CONFIG, READ_WHRM_SAMPLE_RATE, READ_WHRM_SAMPLE_RATE_ID, 2); 
  return sampRate;

}

// Family Byte: READ_ALGORITHM_CONFIG (0x51), Index Byte:
// READ_WHRM_MAX_HEIGHT (0x02), Write Byte: READ_WHRM_MAX_HEIGHT_ID (0x01)
// This function reads the maximum height for the wrist heart rate monitor
// (WHRM) algorithm. 
uint16_t SparkFun_Bio_Sensor_Hub::readWHRMMaxHeight() {

  uint16_t maxHeight = readIntByte(READ_ALGORITHM_CONFIG, READ_WHRM_MAX_HEIGHT, READ_WHRM_MAX_HEIGHT_ID, 3); 
  return maxHeight; 

}

// Family Byte: READ_ALGORITHM_CONFIG (0x51), Index Byte:
// READ_WHRM_MAX_WEIGHT (0x02), Write Byte: READ_WHRM_MAX_WEIGHT_ID (0x02)
// This function reads the maximum weight for the wrist heart rate monitor
// (WHRM) algorithm. 
uint16_t SparkFun_Bio_Sensor_Hub::readWHRMMaxWeight() {

  uint16_t maxWeight = readIntByte(READ_ALGORITHM_CONFIG, READ_WHRM_MAX_WEIGHT, READ_WHRM_MAX_WEIGHT_ID, 3); 
  return maxWeight; 

}

// Family Byte: READ_ALGORITHM_CONFIG (0x51), Index Byte:
// READ_WHRM_MAX_AGE (0x02), Write Byte: READ_MAX_AGE_ID (0x03)
// This function reads the maximum age for the wrist heart rate monitor
// (WHRM) algorithm. 
uint8_t SparkFun_Bio_Sensor_Hub::readWHRMMaxAge() {

  uint8_t maxAge = readByte(READ_ALGORITHM_CONFIG, READ_WHRM_MAX_AGE, READ_WHRM_MAX_AGE_ID, 3); 
  return maxAge; 

}

// Family Byte: READ_ALGORITHM_CONFIG (0x51), Index Byte:
// READ_WHRM_MIN_HEIGHT (0x02), Write Byte: READ_WHRM_MIN_HEIGHT_ID (0x04)
// This function reads the minimum height for the wrist heart rate monitor
// (WHRM) algorithm. 
uint16_t SparkFun_Bio_Sensor_Hub::readWHRMMinHeight() {

  uint16_t minAge = readIntByte(READ_ALGORITHM_CONFIG, READ_WHRM_MIN_HEIGHT, READ_WHRM_MIN_HEIGHT_ID, 3); 
  return minAge; 

}

// Family Byte: READ_ALGORITHM_CONFIG (0x51), Index Byte:
// READ_WHRM_MIN_WEIGHT (0x02), Write Byte: READ_WHRM_MIN_WEIGHT_ID (0x05)
// This function reads the minimum weight for the wrist heart rate monitor
// (WHRM) algorithm. 
uint16_t SparkFun_Bio_Sensor_Hub::readWHRMMinWeight() {

  uint16_t minWeight = readIntByte(READ_ALGORITHM_CONFIG, READ_WHRM_MIN_WEIGHT, READ_WHRM_MIN_WEIGHT_ID, 3); 
  return minWeight;

}

// Family Byte: READ_ALGORITHM_CONFIG (0x51), Index Byte:
// READ_WHRM_MIN_AGE (0x02), Write Byte: READ_WHRM_MIN_AGE_ID (0x06)
// This function reads the minimum age for the wrist heart rate monitor
// (WHRM) algorithm. 
uint8_t SparkFun_Bio_Sensor_Hub::readWHRMMinAge() {

  uint8_t minAge = readByte(READ_ALGORITHM_CONFIG, READ_WHRM_MIN_AGE, READ_WHRM_MIN_AGE_ID, 2); 
  return minAge;

}

// Family Byte: READ_ALGORITHM_CONFIG (0x51), Index Byte:
// READ_WHRM_DEF_HEIGHT (0x02), Write Byte: READ_WHRM_DEF_HEIGHT_ID (0x07)
// This function reads the default height for the wrist heart rate monitor
// (WHRM) algorithm. 
uint16_t SparkFun_Bio_Sensor_Hub::readWHRMDefHeight() {

  uint16_t defHeight = readIntByte(READ_ALGORITHM_CONFIG, READ_WHRM_DEF_HEIGHT, READ_WHRM_DEF_HEIGHT_ID, 3); 
  return defHeight; 

}

// Family Byte: READ_ALGORITHM_CONFIG (0x51), Index Byte:
// READ_WHRM_DEFAULT_WEIGHT (0x02), Write Byte: READ_WHRM_DEF_WEIGHT_ID (0x08)
// This function reads the default weight for the wrist heart rate monitor
// (WHRM) algorithm. 
uint16_t SparkFun_Bio_Sensor_Hub::readWHRMDefWeight() {

  uint16_t defWeight = readIntByte(READ_ALGORITHM_CONFIG, READ_WHRM_DEF_WEIGHT, READ_WHRM_DEF_WEIGHT_ID, 3); 
  return defWeight;

}

// Family Byte: READ_ALGORITHM_CONFIG (0x51), Index Byte:
// READ_WHRM_DEFAULT_AGE (0x02), Write Byte: READ_WHRM_DEF_AGE_ID (0x09)
// This function returns the default age for the wrist heart rate monitor
// (WHRM) algorithm. 
uint8_t SparkFun_Bio_Sensor_Hub::readWHRMDefAge() {

  uint8_t defAge = readByte(READ_ALGORITHM_CONFIG, READ_WHRM_DEF_AGE, READ_WHRM_DEF_AGE_ID, 2); 
  return defAge;

}

// Family Byte: READ_ALGORITHM_CONFIG (0x51), Index Byte:
// READ_WHRM_INIT_HR (0x02), Write Byte: READ_WHRM_INIT_HR_ID (0x0A)
// This function reads the maximum age for the wrist heart rate monitor
// (WHRM) algorithm. 
uint8_t SparkFun_Bio_Sensor_Hub::readWHRMBPM() {

  uint8_t bpm = readByte(READ_ALGORITHM_CONFIG, READ_WHRM_INIT_HR, READ_WHRM_INIT_HR_ID, 2); 
  return bpm; 

}

// Family Byte: READ_ALGORITHM_CONFIG (0x51), Index Byte:
// READ_MAX_FAST_COEF (0x02), Write Byte: READ_MAX_FAST_COEF_ID (0x0B)
// This function reads the maximum age for the wrist heart rate monitor
// (WHRM) algorithm. It returns three long integers that are 
// multiplied by 100,000.
signed long*  SparkFun_Bio_Sensor_Hub::readWHRMCoef(signed long coefArr[3]) {
 
  uint8_t numOfReads = 12; // 3 coefficients * 4 bytes  
  readMultipleBytes( READ_ALGORITHM_CONFIG, READ_MAX_FAST_COEF, READ_MAX_FAST_COEF_ID, numOfReads, coefArr ); 
  coefArr[0] = coefArr[0] * 100000; 
  coefArr[1] = coefArr[1] * 100000; 
  coefArr[2] = coefArr[2] * 100000; 
  return coefArr; 
}

// Family Byte: READ_ALGORITHM_CONFIG (0x51), Index Byte: 
// READ_WHRM_AEC_EN (0x02), Write Byte: READ_WHRM_AEC_EN_ID (0x0B)
// This function reads whether or not the automatic exposure control(AEC) is
// disabled (zero) or enabled (one). 
uint8_t SparkFun_Bio_Sensor_Hub::readAutoExpCont() {
  
  uint8_t enable = readByte( READ_ALGORITHM_CONFIG, READ_WHRM_AEC_EN, READ_WHRM_AEC_EN_ID , 2);
  return enable; 

}

// Family Byte: READ_ALGORITHM_CONFIG (0x51), Index Byte: 
// READ_WHRM_SCD_EN (0x02), Write Byte: READ_WHRM_SCD_EN_ID (0x0C)
// This function reads wehther or not the skin contact detection is enabled or
// disabled. 
uint8_t SparkFun_Bio_Sensor_Hub::readSkinDetect() {
  
  uint8_t enable = readByte( READ_ALGORITHM_CONFIG, READ_WHRM_SCD_EN, READ_WHRM_SCD_EN_ID, 2 );
  return enable; 

}

// Family Byte: READ_ALGORITHM_CONFIG (0x51), Index Byte: 
// READ_WHRM_PD_PRD (0x02), Write Byte: READ_WHRM_PD_PRD_ID (0x0D)
// This function reads the current period of the photo detector in seconds.
uint16_t SparkFun_Bio_Sensor_Hub::readPhotoDetPer() {
  
  uint16_t seconds = readIntByte( READ_ALGORITHM_CONFIG, READ_WHRM_PD_PRD, READ_WHRM_PD_PRD_ID, 3 );
  return seconds; 

}

// Family Byte: READ_ALGORITHM_CONFIG (0x51), Index Byte: 
// READ_WHRM_SCD_DEB (0x02), Write Byte: READ_WHRM_SCD_DEB_ID (0x0E)
// This function reads the skin contract detect debounce window. It's not clear
// if this is in seconds when reading the datasheet. 
uint16_t SparkFun_Bio_Sensor_Hub::readSCDWindow() {
  
  uint16_t seconds = readIntByte( READ_ALGORITHM_CONFIG, READ_WHRM_SCD_DEB, READ_WHRM_SCD_DEB_ID, 3);
  return seconds; 

}

// Family Byte: READ_ALGORITHM_CONFIG (0x51), Index Byte: 
// READ_WHRM_MOT_MAG (0x02), Write Byte: READ_WHRM_MOT_MAG_ID (0x0F)
// This function reads the motion magnitude threshold in 0.1g increments. 
uint16_t SparkFun_Bio_Sensor_Hub::readMotionMag() {
  
  uint16_t thresh = readIntByte( READ_ALGORITHM_CONFIG, READ_WHRM_MOT_MAG, READ_WHRM_MOT_MAG_ID, 3);
  return thresh; 

}

// Family Byte: READ_ALGORITHM_CONFIG (0x51), Index Byte: 
// READ_WHRM_PD_MIN (0x02), Write Byte: READ_WHRM_PD_MIN_ID (0x10)
// This function reads the set minimum photodetector currrent in 0.1mA.
uint16_t SparkFun_Bio_Sensor_Hub::readPDCurrent() {
  
  uint16_t minCurr = readIntByte( READ_ALGORITHM_CONFIG, READ_WHRM_PD_MIN, READ_WHRM_PD_MIN_ID, 3);
  return minCurr; 

}

// Family Byte: READ_ALGORITHM_CONFIG (0x51), Index Byte: 
// READ_WHRM_PD_PPG (0x02), Write Byte: READ_WHRM_PD_PPG_ID (0x11)
// This function reads the current source of the photoplethysmography (PPG) signal for 
// the photodetector (PD). It will return one of three values: zero - PD1, 
// one - PD2, and three - PD1 and PD2.
uint8_t SparkFun_Bio_Sensor_Hub::readPPGSource() {

  uint8_t ppgSource = readByte( READ_ALGORITHM_CONFIG, READ_WHRM_PD_PPG, READ_WHRM_PD_PPG_ID, 2 );
  return ppgSource; 

}

// Family Byte: READ_ALGORITHM_CONFIG (0x51), Index Byte: 
// READ_WSP02_COEF (0x05), Write Byte: READ_WSP02_COEF_ID (0x00)
// This function reads the coefficiencts used for the WSP02 algorithm. It
// returns the three long integers that are multiplied by 100,000 that are used
// as the coefficients. 
signed long* SparkFun_Bio_Sensor_Hub::readWSP02Coef(signed long coefArr[3]) {
  
  uint8_t numOfReads = 12; // 3 coefficients * 4 bytes  
  // The array is populated in the read function. 
  readMultipleBytes( READ_ALGORITHM_CONFIG, READ_WSP02_COEF, READ_WSP02_COEF_ID, numOfReads, coefArr);
  coefArr[0] = coefArr[0] * 100000; 
  coefArr[1] = coefArr[1] * 100000; 
  coefArr[2] = coefArr[2] * 100000; 
  return coefArr; 

}

// Family Byte: READ_ALGORITHM_CONFIG (0x51), Index Byte: 
// READ_WSP02_SAMP_RATE(0x05), Write Byte: READ_WSP02_SAMP_RATE_ID (0x01)
// This function reads the WSP02 sample rate; returned in Hz. 
uint8_t SparkFun_Bio_Sensor_Hub::readWSP02SampRate() {

  uint8_t sampRate = readByte( READ_ALGORITHM_CONFIG, READ_WSP02_SAMP_RATE, READ_WSP02_SAMP_RATE_ID, 2 ); 
  return sampRate;

}

// Family Byte: READ_ALGORITHM_CONFIG (0x51), Index Byte: 
// READ_WSP02_RUN_MODE (0x05), Write Byte: READ_WSP02_RUN_MODE_ID (0x02)
// This function returns the run mode of the WSP02 algorithm: zer0- continuous
// or one - One-shot. 
uint8_t SparkFun_Bio_Sensor_Hub::readWSP02RunMode() {

  uint8_t runMode = readByte( READ_ALGORITHM_CONFIG, READ_WSP02_RUN_MODE, READ_WSP02_RUN_MODE_ID, 2 );  
  return runMode; 

}

// Family Byte: READ_ALGORITHM_CONFIG (0x51), Index Byte: 
// READ_WSP02_AGC_STAT (0x05), Write Byte: READ_WSP02_AGC_STAT_ID (0x03)
// This function reads whether AGC mode is enabled or disabled. 
uint8_t SparkFun_Bio_Sensor_Hub::readAGCmode() {

  uint8_t enable = readByte( READ_ALGORITHM_CONFIG, READ_WSP02_AGC_STAT, READ_WSP02_AGC_STAT_ID, 2 );
  return enable; 

}

// Family Byte: READ_ALGORITHM_CONFIG (0x51), Index Byte: 
// READ_WSP02_MD_STAT (0x05), Write Byte: READ_WSP02_MD_STAT_ID (0x04)
// This function checks whether motion detection is enable (one) or not (zero). 
uint8_t SparkFun_Bio_Sensor_Hub::readMotionDetect() {

  uint8_t motionDetect = readByte( READ_ALGORITHM_CONFIG, READ_WSP02_MD_STAT, READ_WSP02_MD_STAT_ID, 2 );
  return motionDetect; 

}

// Family Byte: READ_ALGORITHM_CONFIG (0x51), Index Byte: 
// READ_WSP02_MD_PRD (0x05), Write Byte: READ_WSP02_MD_PRD (0x05)
// This function reads the motion detection period in seconds. 
uint16_t SparkFun_Bio_Sensor_Hub::readMotionDetecPer() {
  
  uint16_t detPeriod = readIntByte( READ_ALGORITHM_CONFIG, READ_WSP02_MD_PRD, READ_WSP02_MD_PRD_ID, 3 );
  return detPeriod; 

}

// Family Byte: READ_ALGORITHM_CONFIG (0x51), Index Byte: READ_WSP02_MOT_THRESH
// (0x05), Write Byte: READ_WSP02_MOT_THRESH (0x06)
// This function reads the long integer that is the motion threshold times
// 100,000. 
long SparkFun_Bio_Sensor_Hub::readMotThresh() {
  
  long motThresh = readLongByte( READ_ALGORITHM_CONFIG, READ_WSP02_MOT_THRESH, READ_WSP02_MOT_THRESH_ID, 5 );
  return motThresh; 

}


// Family Byte: READ_ALGORITHM_CONFIG (0x51), Index Byte: READ_WSP02_AGC_TO
// (0x05), Write Byte: READ_WSP02_AGC_TO_ID (0x07)
// This function reads the time out period of the AGC for the WSp02 Algorithm. 
uint8_t SparkFun_Bio_Sensor_Hub::readWSP02AGCTimeOut() {

  uint8_t timeOut = readByte( READ_ALGORITHM_CONFIG, READ_WSP02_AGC_TO, READ_WSP02_AGC_TO_ID, 2 );
  return timeOut;

}

// Family Byte: READ_ALGORITHM_CONFIG (0x51), Index Byte: READ_WSP02_ALGTHM_TO 
// (0x05), Write Byte: READ_WSP02_ALGTHM_TO_ID (0x08)
// This function returns the timeout period of the WSp02 Algorithm.
uint8_t SparkFun_Bio_Sensor_Hub::readWSP02AlgTimeOut() {

  uint8_t timeOut = readByte( READ_ALGORITHM_CONFIG, READ_WSP02_ALGTHM_TO, READ_WSP02_ALGTHM_TO_ID, 2 );
  return timeOut; 

}

// Family Byte: READ_ALGORITHM_CONFIG (0x51), Index Byte: READ_WSP02_PD_PPG
// (0x05), Write Byte: READ_WSP02_PD_PPG_ID (0x03)
// This function reads the source of the photoplethysmogorphy: 0x01 = PD1 or
// 0x02 = PD2.  
uint8_t SparkFun_Bio_Sensor_Hub::readWSP02PPGSource() {

  uint8_t ppgSource = readByte( READ_ALGORITHM_CONFIG, READ_WSP02_PD_PPG, READ_WSP02_PD_PPG_ID, 2 );
  return ppgSource;

}

// Family Byte: ENABLE_ALGORITHM (0x52), Index Byte:
// ENABLE_AGC_ALGO (0x00)
// This function enables (one) or disables (zero) the automatic gain control algorithm. 
bool SparkFun_Bio_Sensor_Hub::acgAlgoControl(uint8_t enable) {

  if( enable != 0 || enable != 1)
    return false; 
  
  uint8_t statusByte = writeByte(ENABLE_ALGORITHM, ENABLE_AGC_ALGO, enable);
  if (statusByte == SUCCESS)
    return true;
  else
    return false;

}

// Family Byte: ENABLE_ALGORITHM (0x52), Index Byte:
// ENABLE_AEC_ALGO (0x01)
// This function enables (one) or disables (zero) the automatic exposure
// control (AEC) algorithm.
bool SparkFun_Bio_Sensor_Hub::aecAlgoControl(uint8_t enable) {

  if( enable != 0 || enable != 1)
    return false; 
  
  uint8_t statusByte = writeByte(ENABLE_ALGORITHM, ENABLE_AEC_ALGO, enable);
  if (statusByte == SUCCESS)
    return true;
  else
    return false;

}

// Family Byte: ENABLE_ALGORITHM (0x52), Index Byte:
// ENABLE_WHRM_ALGO (0x02)
// This function enables (one) or disables (zero) the wrist heart rate monitor
// algorithm.
bool SparkFun_Bio_Sensor_Hub::whrmFastAlgoControl(uint8_t algSwitch) {

  if( algSwitch == 0 || algSwitch == 1)
    { }
  else
    return false; 
  
  uint8_t statusByte = writeByte(ENABLE_ALGORITHM, ENABLE_WHRM_ALGO, algSwitch);
  if (statusByte == SUCCESS)
    return true;
  else
    return false;

}

// Family Byte: ENABLE_ALGORITHM (0x52), Index Byte: ENABLE_ECG_ALGO
// (0x03)
// This function enables (one) or disables (zero) the electrocardiogram 
// (ECG) algorithm.
bool SparkFun_Bio_Sensor_Hub::ecgAlgoControl(uint8_t enable) {

  if( enable != 0 || enable != 1)
    return false; 
  
  uint8_t statusByte = writeByte(ENABLE_ALGORITHM, ENABLE_ECG_ALGO, enable);
  if (statusByte == SUCCESS)
    return true;
  else
    return false;

}


// Family Byte: ENABLE_ALGORITHM (0x52), Index Byte: ENABLE_BPT_ALGO
// (0x04)
// This function enables (one) or disables (zero) the blood pressure trending 
// (BPT) algorithm.
bool SparkFun_Bio_Sensor_Hub::bptAlgoControl(uint8_t enable) {

  if( enable == 0 || enable == 1)
  { }
  else
    return false; 
  
  uint8_t statusByte = writeByte(ENABLE_ALGORITHM, ENABLE_BPT_ALGO, enable);
  if (statusByte == SUCCESS)
    return true;
  else
    return false;

}

// Family Byte: ENABLE_ALGORITHM (0x52), Index Byte: ENABLE_WSP02_ALGO
// (0x05)
// This function enables (one) or disables (zero) the WSP02 algorithm..
bool SparkFun_Bio_Sensor_Hub::wsp02AlgoControl(uint8_t enable) {
  
  if( enable != 0 || enable != 1)
    return false; 
  
  uint8_t statusByte = writeByte(ENABLE_ALGORITHM, ENABLE_WSP02_ALGO, enable);
  if (statusByte == SUCCESS)
    return true;
  else
    return false;

}

// Family Byte: BOOTLOADER_FLASH (0x80), Index Byte: SET_INIT_VECTOR_BYTES (0x00)
//void SparkFun_Bio_Sensor_Hub::setInitBytes

// Family Byte: BOOTLOADER_FLASH (0x80), Index Byte: SET_AUTH_BYTES (0x01)

// Family Byte: BOOTLOADER_FLASH (0x80), Index Byte: SET_NUM_PAGES (0x02),
// Write Bytes: 0x00 - Number of pages at byte 0x44 from .msbl file. 
bool SparkFun_Bio_Sensor_Hub::setNumPages(uint8_t totalPages) {

  uint8_t statusByte = writeByte( BOOTLOADER_FLASH, SET_NUM_PAGES, 0x00, totalPages );
  return statusByte; 

}

// Family Byte: BOOTLOADER_FLASH (0x80), Index Byte: ERASE_FLASH (0x03)
// Returns true on successful communication.
bool SparkFun_Bio_Sensor_Hub::eraseFlash() {

  // This is a unique write in that it does not have a relevant write byte.
  _i2cPort->beginTransmission(_address);
  _i2cPort->write(BOOTLOADER_FLASH);    
  _i2cPort->write(ERASE_FLASH);    
  _i2cPort->endTransmission();
  delay(CMD_DELAY); 

  _i2cPort->requestFrom(_address, 1); 
  uint8_t statusByte = _i2cPort->read(); 
  if( !statusByte ) 
    return true; 
  else 
    return false; 

}

// Family Byte: BOOTLOADER_FLASH (0x80), Index Byte: SEND_PAGE_VALUE (0x04)
// Family Byte: BOOTLOADER_INFO (0x81), Index Byte: BOOTLOADER_VERS (0x00)
version SparkFun_Bio_Sensor_Hub::readBootloaderVers(){

  version booVers; //BOO!
  _i2cPort->beginTransmission(_address);
  _i2cPort->write(BOOTLOADER_INFO);    
  _i2cPort->write(BOOTLOADER_VERS);    
  _i2cPort->endTransmission();
  delay(CMD_DELAY); 

  _i2cPort->requestFrom(_address, 4); 
  uint8_t statusByte = _i2cPort->read();
  if( !statusByte ) // Pass through if SUCCESS (0x00). 
    return; // Just return

  booVers.major = _i2cPort->read();
  booVers.minor = _i2cPort->read();
  booVers.revision = _i2cPort->read();  

  return booVers; 

}

// Family Byte: BOOTLOADER_INFO (0x81), Index Byte: PAGE_SIZE (0x01)
// Family Byte: IDENTITY (0xFF), Index Byte: READ_SENSOR_HUB_VERS (0x03)
version SparkFun_Bio_Sensor_Hub::readSensorHubVersion(){

  version bioHubVers; 
  _i2cPort->beginTransmission(_address);
  _i2cPort->write(BOOTLOADER_INFO);    
  _i2cPort->write(BOOTLOADER_VERS);    
  _i2cPort->endTransmission();
  delay(CMD_DELAY); 

  _i2cPort->requestFrom(_address, 4); 
  uint8_t statusByte = _i2cPort->read();
  if( !statusByte ) // Pass through if SUCCESS (0x00). 
    return; // Just return

  bioHubVers.major = _i2cPort->read();
  bioHubVers.minor = _i2cPort->read();
  bioHubVers.revision = _i2cPort->read();  

  return bioHubVers; 

}

// Family Byte: IDENTITY (0xFF), Index Byte: READ_ALGO_VERS (0x07)
version SparkFun_Bio_Sensor_Hub::readAlgorithmVersion(){

  version libAlgoVers; 
  _i2cPort->beginTransmission(_address);
  _i2cPort->write(BOOTLOADER_INFO);    
  _i2cPort->write(BOOTLOADER_VERS);    
  _i2cPort->endTransmission();
  delay(CMD_DELAY); 

  _i2cPort->requestFrom(_address, 4); 
  uint8_t statusByte = _i2cPort->read();
  if( !statusByte ) // Pass through if SUCCESS (0x00). 
    return; // Just return

  libAlgoVers.major = _i2cPort->read();
  libAlgoVers.minor = _i2cPort->read();
  libAlgoVers.revision = _i2cPort->read();  

  return libAlgoVers; 

}

//-------------------Private Functions-----------------------

// For Read: Family Byte: READ_ALGORITHM_CONFIG (0x51), Index Byte:
// READ_WHRM_BPT_RESULTS (0x04), Write Byte: BPT_CALIBRATE_ID (0x03) 
// For Write: Family Byte: CHANGE_ALGORITHM_CONFIG (0x50), Index Byte: CALIBRATE_BPT
// (0x04), Write Byte: BPT_CALIBRATE_ID (0x03)
// This function takes the 608 data points acquired by the calibration
// procedure and feeds them into the blood pressure trending
// algorithm. 
bool SparkFun_Bio_Sensor_Hub::calibrateBPTAlm(){
 
  uint8_t statusByte; 
  _i2cPort->beginTransmission(_address);     
  _i2cPort->write(READ_ALGORITHM_CONFIG);    
  _i2cPort->write(READ_WHRM_BPT_RESULTS);    
  _i2cPort->write(READ_WHRM_BPT_RESULTS_ID); 
  _i2cPort->endTransmission(); 
  delay(CMD_DELAY); 

  _i2cPort->requestFrom(_address, 609); // 608 bytes of data and one status byte
  statusByte = _i2cPort->read();  
  if( statusByte != SUCCESS )
    return false; 
  else {
    for( byte i = 0; i < 608; i++ ){ // Read all data into the calibration data array
      _calibData[i] = _i2cPort->read(); 
    }
  }

  _i2cPort->beginTransmission(_address);     
  _i2cPort->write(CHANGE_ALGORITHM_CONFIG);    
  _i2cPort->write(CALIBRATE_BPT);    
  _i2cPort->write(BPT_CALIBRATE_ID); 
  for(byte i = 0; i < 608; i++){ // Length of the given calibration data is 608
    _i2cPort->write(_calibData[i]);     
  }
  _i2cPort->endTransmission(); 
  delay(CMD_DELAY); 
  _i2cPort->requestFrom(_address, 1); // 608 bytes of data and one status byte
  statusByte = _i2cPort->read();  
  if( statusByte != SUCCESS )
    return false; 
}


// This function uses the given family, index, and write byte to communicate
// with the MAX32664 which in turn communicates with downward sensors. There
// are two steps demonstrated in this function. First a write to the MCU
// indicating what you want to do, a delay, and then a read to confirm positive
// transmission. 
uint8_t SparkFun_Bio_Sensor_Hub::writeByte(uint8_t _familyByte, uint8_t _indexByte, uint8_t _writeByte) {

  _i2cPort->beginTransmission(_address);     
  _i2cPort->write(_familyByte);    
  _i2cPort->write(_indexByte);    
  _i2cPort->write(_writeByte); 
  _i2cPort->endTransmission(); 
  delay(500); 

  _i2cPort->requestFrom(_address, 1); // Status Byte, success or no? 0x00 is a successful transmit
  uint8_t statusByte = _i2cPort->read(); 
  return statusByte; 

}

// This function is the same as the function above and uses the given family, 
// index, and write byte, but also takes a 16 bit integer as a paramter to communicate
// with the MAX32664 which in turn communicates with downward sensors. There
// are two steps demonstrated in this function. First a write to the MCU
// indicating what you want to do, a delay, and then a read to confirm positive
// transmission. 
uint8_t SparkFun_Bio_Sensor_Hub::writeByte(uint8_t _familyByte, uint8_t _indexByte, uint8_t _writeByte, uint16_t _val) {

  _i2cPort->beginTransmission(_address);     
  _i2cPort->write(_familyByte);    
  _i2cPort->write(_indexByte);    
  _i2cPort->write(_writeByte); 
  _i2cPort->write((_val >> 8)); // MSB
  _i2cPort->write(_val);  // LSB
  _i2cPort->endTransmission(); 
  delay(CMD_DELAY); 

  _i2cPort->requestFrom(_address, 1); // Status Byte, success or no? 0x00 is a successful transmit
  uint8_t statusByte = _i2cPort->read(); 
  return statusByte; 

}

// This function sends information to the MAX32664 to specifically write values
// to the registers of downward sensors and so also requires a
// register address and register value as parameters. Again there is the write
// of the specific bytes followed by a read to confirm positive transmission. 
uint8_t SparkFun_Bio_Sensor_Hub::writeByte(uint8_t _familyByte, uint8_t _indexByte, uint8_t _writeByte, uint8_t _writeVal)
{

  _i2cPort->beginTransmission(_address);     
  _i2cPort->write(_familyByte);    
  _i2cPort->write(_indexByte);    
  _i2cPort->write(_writeByte);    
  _i2cPort->write(_writeVal);    
  _i2cPort->endTransmission(); 
  delay(CMD_DELAY); 

  _i2cPort->requestFrom(_address, 1); // Status Byte, 0x00 is a successful transmit.
  uint8_t statusByte = _i2cPort->read(); 
  return statusByte; 

}

// This function sends information to the MAX32664 to specifically write values
// to the registers of downward sensors and so also requires a
// register address and register value as parameters. Again there is the write
// of the specific bytes followed by a read to confirm positive transmission. 
uint8_t SparkFun_Bio_Sensor_Hub::writeLongBytes(uint8_t _familyByte, uint8_t _indexByte, uint8_t _writeByte, long _writeVal[3])
{

  _i2cPort->beginTransmission(_address);     
  _i2cPort->write(_familyByte);    
  _i2cPort->write(_indexByte);    
  _i2cPort->write(_writeByte);    
  for( byte i = 0; i < 3; i++){
    _i2cPort->write(_writeVal[i] >> 24); 
    _i2cPort->write(_writeVal[i] >> 16); 
    _i2cPort->write(_writeVal[i] >> 8); 
    _i2cPort->write(_writeVal[i]); 
  }
  _i2cPort->endTransmission(); 
  delay(CMD_DELAY); 

  _i2cPort->requestFrom(_address, 1); // Status Byte, 0x00 is a successful transmit.
  uint8_t statusByte = _i2cPort->read(); 
  return statusByte; 

}

// This function handles all read commands or stated another way, all information
// requests. It starts a request by writing the family byte an index byte, and
// then delays 60 microseconds, during which the MAX32664 retrieves the requested 
// information. An I-squared-C request is then issued, and the information is read.
uint8_t SparkFun_Bio_Sensor_Hub::readByte(uint8_t _familyByte, uint8_t _indexByte, uint8_t _numOfReads )
{

  uint8_t returnByte;
  uint8_t statusByte;
  _i2cPort->beginTransmission(_address);
  _i2cPort->write(_familyByte);    
  _i2cPort->write(_indexByte);    
  _i2cPort->endTransmission();
  delay(CMD_DELAY);
  
  _numOfReads++; // Status byte.... 
  _i2cPort->requestFrom(_address, _numOfReads); 
  statusByte = _i2cPort->read();
  _numOfReads--; // One read for status byte.  
  if( statusByte )// SUCCESS (0x00) - how do I know its 
    return statusByte; // Return the error, see: READ_STATUS_BYTE_VALUE 

  for(uint8_t i = 0; i < _numOfReads; i++){
    returnByte = _i2cPort->read(); 
  }
  return returnByte; // If good then return the actual byte. 



}

// This function is exactly as the one above except it accepts also receives a 
// Write Byte as a paramter. It starts a request by writing the family byte, index byte, and
// write byte to the MAX32664 and then delays 60 microseconds, during which
// the MAX32664 retrieves the requested information. A I-squared-C request is
// then issued, and the information is read.
uint8_t  SparkFun_Bio_Sensor_Hub::readByte(uint8_t _familyByte, uint8_t _indexByte, uint8_t _writeByte, uint8_t _numOfReads )
{

  uint8_t returnByte;
  uint8_t statusByte;
  _i2cPort->beginTransmission(_address);
  _i2cPort->write(_familyByte);    
  _i2cPort->write(_indexByte);    
  _i2cPort->write(_writeByte);    
  _i2cPort->endTransmission();
  delay(CMD_DELAY); 

  _numOfReads++; // Status byte.... 
  _i2cPort->requestFrom(_address, _numOfReads); 
  statusByte = _i2cPort->read();
  _numOfReads--; // One read for status byte.  
  if( statusByte )// SUCCESS (0x00)
    return statusByte; // Return the error, see: READ_STATUS_BYTE_VALUE 

  for(uint8_t i = 0; i < _numOfReads; i++){
    returnByte = _i2cPort->read(); 
  }
  return returnByte; // If good then return the actual byte. 

}

uint8_t* SparkFun_Bio_Sensor_Hub::readFillArray(uint8_t _familyByte, uint8_t _indexByte, uint8_t _numOfReads, uint8_t array[] )
{

  uint8_t returnByte;
  uint8_t statusByte;
  _i2cPort->beginTransmission(_address);
  _i2cPort->write(_familyByte);    
  _i2cPort->write(_indexByte);    
  _i2cPort->endTransmission();
  delay(CMD_DELAY); 

  _numOfReads++; // Add one for statusByte 
  _i2cPort->requestFrom(_address, _numOfReads); 
  statusByte = _i2cPort->read(); // Got it
  _numOfReads--; // One read for status byte, remove one from reads.   
  if( statusByte )// SUCCESS (0x00)
    return statusByte; // Return the error, see: READ_STATUS_BYTE_VALUE 

  for(uint8_t i = 0; i < _numOfReads; i++){
    array[i] = _i2cPort->read(); 
  }
  return array; // If good then return the array. 

}
// This function handles all read commands or stated another way, all information
// requests. It starts a request by writing the family byte, an index byte, and
// a write byte and then then delays 60 microseconds, during which the MAX32664 
// retrieves the requested information. An I-squared-C request is then issued, 
// and the information is read. This differs from the above read commands in
// that it returns a 16 bit integer instead of 8. 
uint16_t SparkFun_Bio_Sensor_Hub::readIntByte(uint8_t _familyByte, uint8_t _indexByte, uint8_t _writeByte, uint8_t _numOfReads )
{

   uint16_t returnByte;
   uint16_t statusByte; 
  _i2cPort->beginTransmission(_address);
  _i2cPort->write(_familyByte);    
  _i2cPort->write(_indexByte);    
  _i2cPort->write(_writeByte);    
  _i2cPort->endTransmission();
  delay(CMD_DELAY); 

  _numOfReads++; // Status byte.... 
  _i2cPort->requestFrom(_address, _numOfReads); 
  statusByte = _i2cPort->read();
  _numOfReads--; // One read for status byte.  
  if( statusByte ) // Pass through if SUCCESS (0x00). 
    return statusByte; // Return the error, see: READ_STATUS_BYTE_VALUE 

  for(uint8_t i = 0; i < _numOfReads; i++){
    returnByte |= (_i2cPort->read() << 8);
    returnByte |= _i2cPort->read();
  }
  return returnByte; 

}

// This function handles all read commands or stated another way, all information
// requests. It starts a request by writing the family byte, an index byte, and
// a write byte and then then delays 60 microseconds, during which the MAX32664 
// retrieves the requested information. An I-squared-C request is then issued, 
// and the information is read. This differs from the above read commands in
// that it returns a 4 byte (long) integer instead of 8. 
long SparkFun_Bio_Sensor_Hub::readLongByte(uint8_t _familyByte, uint8_t _indexByte, uint8_t _writeByte, uint8_t _numOfReads )
{

   long returnByte;
   long statusByte; 
  _i2cPort->beginTransmission(_address);
  _i2cPort->write(_familyByte);    
  _i2cPort->write(_indexByte);    
  _i2cPort->write(_writeByte);    
  _i2cPort->endTransmission();
  delay(CMD_DELAY); 

  _numOfReads++; // Status byte.... 
  _i2cPort->requestFrom(_address, _numOfReads); 
  statusByte = _i2cPort->read();
  _numOfReads--; // One read for status byte.  
  if( statusByte ) // Pass through if SUCCESS (0x00). 
    return statusByte; // Return the error, see: READ_STATUS_BYTE_VALUE 

  for(uint8_t i = 0; i < _numOfReads; i++){
    returnByte |= (_i2cPort->read() << 24);
    returnByte |= (_i2cPort->read() << 16);
    returnByte |= (_i2cPort->read() << 8);
    returnByte |= _i2cPort->read();
  }
  return returnByte; 

}
// This function handles all read commands or stated another way, all information
// requests. It starts a request by writing the family byte, an index byte, and
// a write byte and then then delays 60 microseconds, during which the MAX32664 
// retrieves the requested information. An I-squared-C request is then issued, 
// and the information is read. This function is very similar to the one above
// except it returns three long bytes instead of one. 
long* SparkFun_Bio_Sensor_Hub::readMultipleBytes(uint8_t _familyByte, uint8_t _indexByte, uint8_t _writeByte, uint8_t _numOfReads, signed long* array )
{

   uint8_t statusByte; 
  _i2cPort->beginTransmission(_address);
  _i2cPort->write(_familyByte);    
  _i2cPort->write(_indexByte);    
  _i2cPort->write(_writeByte);    
  _i2cPort->endTransmission();
  delay(CMD_DELAY); 

  _numOfReads++; // Status byte.... 
  _i2cPort->requestFrom(_address, _numOfReads); 
  statusByte = _i2cPort->read();
  _numOfReads--; // One read for status byte.  
  if( statusByte ) // Pass through if SUCCESS (0x00). 
    return; 

  for(uint8_t i = 0; i < _numOfReads; i++){
    array[i] |= (_i2cPort->read() << 24);
    array[i] |= (_i2cPort->read() << 16);
    array[i] |= (_i2cPort->read() << 8);
    array[i] |= _i2cPort->read();
  }
  return array; 

}
