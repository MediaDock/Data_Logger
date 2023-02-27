/*
                #####                                                           
              ##########                                                        
             ###     ###                                                        
             ###     ###                                                        
             ###     ###                                                        
             ###     ###                                                        
             ###     ###                                                
             ###     ##########                                                 
             ###     ##################                                        
             ###     ###     ##############                                   
             ###     ###     ####     #######                               
             ###     ###     ####     ###   ###                                
  ######     ###     ###     ####     ###   ######                            
###########  ###     ###     ####     ###       ###                            
####     #######                      ###       ###                            
#######      ###                                 ###                            
  #####                                          ###                            
  #######                                        ###                            
     ####                                        ###                            
      ######                                     ###                            
         ###                                  #####                            
         #######                             ####                               
            ####                             ###                                                         
             #######                     #######
                ####                     ####                                   
                #############################                                   
                #############################                                   
                Hardware used: Seeeduino xiao & its internal RTC                
                Built for a Workshop at MediaDock HSLU D&K
                Using the internals RTC of Seeedstudio xiao 
                It puts the Xiao into DeepSleep between Sensorreadings and Data Storage.
                But the Timestamp starts at 0:0:0 each time the System is powered up...
                If you liike tho have the accurate Time and Date use a RTC module like DS3231 

                In this Example we are using the BME 280 Airpressure Sensor and a Capacitattive Soil moisture Sensor
*/




int MakeABreak = 600;  // YOUR CODE HERE: write every ... seconds this sets the interval

/* Capacitative Soil Moisture Sensor ***************************************************/
int MoisturePin = A0; // SensorPin of your MoistureSensor 
int PowerPIN = 6; // MoistureSensor PowerPin (V)

const int dry = 600;  // YOUR CODE HERE: value for dry sensor
const int wet = 200;  // YOUR CODE HERE: value for wet sensor
char SensorReading1[60];
char SensorReading2[60];

/* Realtime Clock Globals **************************************************************/
#include <RTCCounter.h> // version 1.0.1
#include <time.h>
char DateTimeBuffer[32];

/* SD Card Globals **********************************************************************/
// ATTENTION: SD Card max Size: 16GB ---> FAT16 or FAT32 Formatted

#include <SD.h> // version 1.2.4 
#include "SPI.h"
File myFile;
char filename[16];  // make it long enough to hold your longest file name, plus a null terminator
const int chipSelect = 3;
String DataString = "";  // holds the data to be written to the SD card



/* BME 280 Globales ********************************************************************/
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Wire.h>
#define SEALEVELPRESSURE_HPA (1013.25)
Adafruit_BME280 bme;  // I2C
int TEMP = 0;
float hPa = 0;
int HUMID = 0;
unsigned long lastBME = 0UL;


/****************************************************************************************/
/* SETUP ********************************************************************************/
/****************************************************************************************/

void setup() {
  // ATTENTION: Sleep mode can intefere with reprogramming.
  // A startup delay makes things easier as it
  // provides a window to upload new code

  // If you get stuck and want to reset the Seeed Studio XIAO SAMD21 , perform the following steps:
  // Connect the Seeed Studio XIAO SAMD21 to your computer.
  // Use tweezers or short lines to short the RST pins only once
  // The orange LED lights flicker on and light up.

  delay(10000);

  Serial.begin(57600);
  //Activating the PowerPIN to power your Sensor only at work
  pinMode(PowerPIN, OUTPUT);  

  /**SD Card Module Init *******************************************/
  Serial.print("Initializing SD card...");
  if (!SD.begin(chipSelect)) {
    Serial.println("SD Card initialization failed!");
    delay(2000);
    // ATTENTION: THE LOGGERR ONLY WORKS IF AN SD CARD IS DETECTED IN THE SDCARD READER
    return;  
  }
  Serial.println("SD card initialized.");


  /**File Initialization*******************************************/
  int n = 0;
  // includes a three-digit sequence number in the file name
  snprintf(filename, sizeof(filename), "Data%03d.csv", n);  
  while (SD.exists(filename)) {
    n++;
    delay(100);
    snprintf(filename, sizeof(filename), "Data%03d.csv", n);
  }
  if (myFile) {
    // always close the file    
    myFile.close(); 
  }

  Serial.print("new File initialized: ");
  Serial.println(F(filename));


  /**File Categories Initialization *******************************/

  File myFile = SD.open(filename, FILE_WRITE);
    if(SD.exists(filename)) { // check the card is still there
      myFile = SD.open(filename, FILE_WRITE); // now append new data file
      if (myFile){
        myFile.println("Date,Time,SoilHumidity RAW,SoilHumidity %,Temperature C°,AirPressure hPa, AirHumidity %");
        myFile.close(); // close the file
        Serial.println("Categories saved to File: Date,Time,SoilHumidity RAW,SoilHumidity %,Temperature C°,AirPressure hPa, AirHumidity %");
      }
    }
    else{
        Serial.println("Error writing to file !");
    }

  /*RTC Initialization **********************************************/
  rtcCounter.begin();
  // Set the alarm to generate an interrupt every 5s
  rtcCounter.setPeriodicAlarm(MakeABreak);
  // Set the start date and time
  setDateTime(2023, 2, 27, 0, 0, 0);
  // Set the sleep mode
  SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;

  /*BME Initialization **********************************************/

  if (!bme.begin(0x76, &Wire)) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1)
      ;
  }

  /* SETUP COMLETED -> LED Status Light for completed Setup ***************************************/
  Serial.println("Setup Completed!");

  // Blink Onboard LED 10 Times if everything Initialized properly 
  for (int i = 0; i <= 10; i++) {
  pinMode(LED_BUILTIN, OUTPUT);  
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
  }
}

/****************************************************************************************/
/* LOOP ********************************************************************************/
/****************************************************************************************/


void loop() {
  // If the alarm interrupt has been triggered
  if (rtcCounter.getFlag()) {
    
    // Clear the interrupt flag
    rtcCounter.clearFlag();
    digitalWrite(PowerPIN, HIGH);
    delay(100);
    GetSensorData();
    GetDataString();
    SaveData();
    digitalWrite(PowerPIN, LOW);
    delay(100);
  }
  // Sleep until the next interrupt
  systemSleep();
}


/****************************************************************************************/
/* GET DATA FROM SENSOR FUNCTION ********************************************************/
/****************************************************************************************/

void GetSensorData() {

/* Soil Moisture Sensor ****************************************************************/

    int sensorVal = analogRead(MoisturePin);
    int percentageHumidity = map(sensorVal, wet, dry, 100, 0); 
    sprintf(SensorReading1, "%d,%d,", sensorVal, percentageHumidity);
    // Serial.println(SensorReading1);        

/* BME 280 Sensor **********************************************************************/
    bme.takeForcedMeasurement();
    TEMP = bme.readTemperature();
    hPa = (bme.readPressure() / 100.0F);
    HUMID = bme.readHumidity();


    char hPaString [6];
    
    dtostrf(hPa, 6, 2, hPaString);
    sprintf(SensorReading2, "%d,%s,%i ", TEMP, hPaString, HUMID);
 //   Serial.println(SensorReading2);
  
  }


/****************************************************************************************/
/* BUILD DATA STRING FUNCTION ***********************************************************/
/****************************************************************************************/

void GetDataString() {

  printDateTime();
  DataString = String(DateTimeBuffer) + SensorReading1;
  DataString = DataString + SensorReading2;
  Serial.println(DataString);
}


/****************************************************************************************/
/* SAVE DATA FUNCTION *******************************************************************/
/****************************************************************************************/

void SaveData() {
  if (SD.exists(filename)) {                 
    // check the card is still there
    myFile = SD.open(filename, FILE_WRITE);  
    // now append new data file
    if (myFile) {
      myFile.println(DataString);
      // always close the file
      myFile.close();  
      Serial.println("Data saved to SD Card");
    }
  } else {
    Serial.println("Error writing to file !");
  }
}

/****************************************************************************************/
/* SYSTEM SLEEP FUNCTION ****************************************************************/
/****************************************************************************************/

void systemSleep() {
  
  // If the alarm interrupt has not yet triggered
  if (!rtcCounter.getFlag()) {

    // Wait For Interrupt
    __WFI();
  }
}

/****************************************************************************************/
/* SET CLOCK FUNCTION *******************************************************************/
/****************************************************************************************/

// Sets the clock based on date and time
void setDateTime(uint16_t year, uint8_t month, uint8_t day,
                 uint8_t hour, uint8_t minute, uint8_t second) {
  // Use the tm struct to convert the parameters to and from an epoch value
  struct tm tm;

  tm.tm_isdst = -1;
  tm.tm_yday = 0;
  tm.tm_wday = 0;
  tm.tm_year = year - 1900;
  tm.tm_mon = month - 1;
  tm.tm_mday = day;
  tm.tm_hour = hour;
  tm.tm_min = minute;
  tm.tm_sec = second;

  rtcCounter.setEpoch(mktime(&tm));
}

/****************************************************************************************/
/* PRINT TIME FUNCTION ******************************************************************/
/****************************************************************************************/

// Prints out the date and time to SerialUSB
void printDateTime() {
  // Get time as an epoch value and convert to tm struct
  time_t epoch = rtcCounter.getEpoch();
  struct tm* t = gmtime(&epoch);
  // Format and print the output
  snprintf(DateTimeBuffer, sizeof(DateTimeBuffer), "%02d.%02d.%02d,%02d:%02d:%02d,", t->tm_mday, t->tm_mon + 1, t->tm_year - 100,
           t->tm_hour, t->tm_min, t->tm_sec);
}