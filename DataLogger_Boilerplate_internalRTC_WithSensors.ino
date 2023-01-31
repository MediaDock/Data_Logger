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
                Using the internals RTC of Seeedstudio xiao is mor Enegry efficient
                It puts the Xiao into DeepSleep between Sensorreadings and Data Storage.
                But the Timestamp starts at 0:0:0 each time the System is powered up...
                If you liike tho have the accurate Time and Date use a RTC module like DS3231 

                In this Example we are using the BME 280 Airpressure Sensor and a Capacitattive Soil moisture Sensor
*/

int MakeABreak = 5; // write every ... Seconds 

/* Realtime Clock Globals **************************************************************/
#include <RTCCounter.h>
#include <time.h>
char DateTimeBuffer[32];

/* SD Card Globals **********************************************************************/
// SD Card max Size: 16GB ---> FAT16 or FAT32 Formatted 

#include <SD.h>
#include"SPI.h"
File myFile;
char filename[16]; // make it long enough to hold your longest file name, plus a null terminator
const int chipSelect = 3;
String DataString =""; // holds the data to be written to the SD card


/* Capacitative Soil Moisture Sensor ***************************************************/
const int dry = 770; // value for dry sensor
const int wet = 315; // value for wet sensor
char SensorReading1[60];
char SensorReading2[60];


/* BME 280 Globales ********************************************************************/
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#define SEALEVELPRESSURE_HPA (1013.25)
Adafruit_BME280 bme; // I2C
int     TEMP        = 0;
float   hPa         = 0;  
int     HUMID       = 0;
unsigned long lastBME = 0UL;


/****************************************************************************************/
/* SETUP ********************************************************************************/
/****************************************************************************************/



void setup()
{
  // Sleep mode can intefere with reprogramming.
  // A startup delay makes things easier as it
  // provides a window to upload new code 
      // If you want to reset the Seeed Studio XIAO SAMD21 , perform the following steps:
      // Connect the Seeed Studio XIAO SAMD21 to your computer.
      // Use tweezers or short lines to short the RST pins only once
      // The orange LED lights flicker on and light up.
 
  delay(5000);

    Serial.begin(57600);
    while (! Serial); 
  
  /**SD Card Module Init *******************************************/
    Serial.print("Initializing SD card..."); 
      if (!SD.begin(chipSelect)) {
        Serial.println("SD Card initialization failed!");
        delay(2000);
        return; // don't do anything more: THE LOGGERR ONLY WORKS IF AN SD CARD IS DETECTED IN THE SDCARD READER
      }
    Serial.println("SD card initialized.");


  /**File Initialization*******************************************/

    int n = 0;
    snprintf(filename, sizeof(filename), "Data%03d.csv", n); // includes a three-digit sequence number in the file name
    
    while (SD.exists(filename)) {
      n++;
      delay(100);
      snprintf(filename, sizeof(filename), "Data%03d.csv", n);
    }

    File myFile = SD.open(filename, FILE_WRITE);

    if (myFile) {
      myFile.close(); // close the file
    }

    Serial.print("new File initialized: ");
    Serial.println(F(filename));

  /*RTC Initialization **********************************************/
  
  // Setup the RTCCounter
  rtcCounter.begin();

  // Set the alarm to generate an interrupt every 5s
  rtcCounter.setPeriodicAlarm(MakeABreak);

  // Set the start date and time
  setDateTime(2023, 1, 31, 0, 0, 0);

  // Set the sleep mode
  SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;

  /*BME Initialization **********************************************/
   
   unsigned status;
   status = bme.begin(0x76);  
   
   if (!status) {
        Serial.println(F("BME280 allocation failed"));        
    }
  
  Serial.println("Setup Completed!");

}

/****************************************************************************************/
/* LOOP ********************************************************************************/
/****************************************************************************************/


void loop () {
  // If the alarm interrupt has been triggered
  if (rtcCounter.getFlag()) {

    // Clear the interrupt flag
    rtcCounter.clearFlag(); 
    GetSensorData();
    GetDataString();
    SaveData();
  }

  // Sleep until the next interrupt
  systemSleep();
}


/****************************************************************************************/
/* GET DATA FROM SENSOR FUNCTION ********************************************************/
/****************************************************************************************/

void GetSensorData(){

/* Soil Moisture Sensor ****************************************************************/

   int sensorVal = analogRead(A0);
   int percentageHumidity = map(sensorVal, wet, dry, 100, 0); 
    

    sprintf(SensorReading1, "SoilHumidity_RAW:, %d , SoilHumidityPercentage:, %d ,", sensorVal, percentageHumidity);
    // Serial.println(SensorReading1);        

/* BME 280 Sensor **********************************************************************/
    TEMP = bme.readTemperature();
    hPa = (bme.readPressure() / 100.0F);
    HUMID = bme.readHumidity();


    char hPaString [6];
    
    dtostrf(hPa, 6, 2, hPaString);
    sprintf(SensorReading2, "Temperature:, %d ,C°, %s ,hPa, AirHumidity:, %i ", TEMP, hPaString, HUMID);
 //   Serial.println(SensorReading2);
          
}


/****************************************************************************************/
/* BUILD DATA STRING FUNCTION ***********************************************************/
/****************************************************************************************/

void GetDataString(){
    printDateTime();

    DataString = String(DateTimeBuffer) + SensorReading1;
    DataString = DataString + SensorReading2;
    Serial.println(DataString);             
    }


/****************************************************************************************/
/* SAVE DATA FUNCTION *******************************************************************/
/****************************************************************************************/

void SaveData(){
    if(SD.exists(filename)) { // check the card is still there
      myFile = SD.open(filename, FILE_WRITE); // now append new data file
      if (myFile){
        myFile.println(DataString);
        myFile.close(); // close the file
        Serial.println("Data saved to SD Card");
      }
    }
    else{
        Serial.println("Error writing to file !");
    }
}

/****************************************************************************************/
/* SYSTEM SLEEP FUNCTION ****************************************************************/
/****************************************************************************************/

void systemSleep()
{
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
  uint8_t hour, uint8_t minute, uint8_t second)
{
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
void printDateTime()
{
  // Get time as an epoch value and convert to tm struct
  time_t epoch = rtcCounter.getEpoch();
  struct tm* t = gmtime(&epoch);
  // Format and print the output
  snprintf(DateTimeBuffer, sizeof(DateTimeBuffer), "%02d.%02d.%02d, %02d:%02d:%02d,", t->tm_mday, t->tm_mon + 1, t->tm_year - 100, 
      t->tm_hour, t->tm_min, t->tm_sec);

}
