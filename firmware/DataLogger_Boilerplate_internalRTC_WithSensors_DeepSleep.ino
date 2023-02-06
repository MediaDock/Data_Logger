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



/* Realtime RTCzero Globals ************************************************************/
/*  NOTE:
  If you use this sketch you will see no output on the serial monitor.
  This happens because the USB clock is stopped so it the USB connection is stopped too.
*/
#include <RTCZero.h>
int Alarm = 00; // for an alarm each time the rtc reaches 00 seconds
RTCZero rtc;    // Create an rtc object 
char DateTimeBuffer[32];

// Change these values to set the current initial time 
int seconds = 0;
int minutes = 00;
int hours = 17;

// Change these values to set the current initial date 
int day = 3;
int month = 2;
int year = 23;

/* SD Card Globals **********************************************************************/
// SD Card max Size: 16GB ---> FAT16 or FAT32 Formatted 

#include <SD.h>
#include"SPI.h"
File myFile;
char filename[16]; // make it long enough to hold your longest file name, plus a null terminator
const int chipSelect = 3;
String DataString =""; // holds the data to be written to the SD card


/* Capacitative Soil Moisture Sensor ***************************************************/
const int dry = 600; // value for dry sensor
const int wet = 200; // value for wet sensor
char SensorReading1[60];
char SensorReading2[60];
int PowerPIN = 6;


/* BME 280 Globales ********************************************************************/
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#define SEALEVELPRESSURE_HPA (1013.25)
Adafruit_BME280 bme; // I2C
int     TEMP        = 0;
float   hPa         = 0;  
int     HUMID       = 0;
unsigned long lastBME = 0UL;
unsigned status;


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
 
  delay(10000);

    Serial.begin(57600);
    //while (! Serial); 
    pinMode(PowerPIN,OUTPUT); //Activating The PowerPIN

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



  /*BME Initialization **********************************************/
   
   status = bme.begin(0x76);     
   if (!status) {
        Serial.println(F("BME280 allocation failed"));        
    }
  
  /*RTC Initialization **********************************************/
  


  pinMode(LED_BUILTIN, OUTPUT); // Blink 10 Times if SD Card Initialized properly
    for (int i = 0 ; i <= 10; i++) {
        digitalWrite(LED_BUILTIN, LOW);
        delay(100);
        digitalWrite(LED_BUILTIN, HIGH);
        delay(100);  
      } 

  Serial.println("Setup Completed!");

    rtc.begin();
    rtc.setTime(hours, minutes, seconds);
    rtc.setDate(day, month, year);
    rtc.setAlarmSeconds(00);
    rtc.enableAlarm(rtc.MATCH_SS);
    rtc.attachInterrupt(alarmMatch);
    rtc.standbyMode();

}

/****************************************************************************************/
/* LOOP ********************************************************************************/
/****************************************************************************************/


void loop () {

  rtc.standbyMode(); // THE WHOLE MAGIC HAPPENS WHEN alarmMatch(); is triggered!!! 

}


/****************************************************************************************/
/* alarmMatch ***************************************************************************/
/****************************************************************************************/


void alarmMatch()
{
     for (int i = 0 ; i <= 5; i++) {
          digitalWrite(LED_BUILTIN, LOW);
          delay(100);
          digitalWrite(LED_BUILTIN, HIGH);
          delay(100);  
      } 
    digitalWrite(PowerPIN, HIGH);
    delay(100);  
    GetTimeString();
    GetSensorData();
    GetDataString();
    SaveData();
    digitalWrite(PowerPIN, LOW);
    delay(100); 
}


/****************************************************************************************/
/* PRINT TIME FUNCTION ******************************************************************/
/****************************************************************************************/

// Prints out the date and time to SerialUSB
void GetTimeString()
{
  // Format and print the output
  day = rtc.getDay();
  month = rtc.getMonth();
  year = rtc.getYear();
  hours = rtc.getHours();
  minutes = rtc.getMinutes(); 
  seconds = rtc.getSeconds();

  snprintf(DateTimeBuffer, sizeof(DateTimeBuffer), "%02d.%02d.%02d,%02d:%02d:%02d,", day, month, year, 
      hours, minutes, seconds);
  // Serial.print(DateTimeBuffer);
}


/****************************************************************************************/
/* GET DATA FROM SENSOR FUNCTION ********************************************************/
/****************************************************************************************/

void GetSensorData(){

/* Soil Moisture Sensor ****************************************************************/

   int sensorVal = analogRead(A0);
   int percentageHumidity = map(sensorVal, wet, dry, 100, 0); 
    

    sprintf(SensorReading1, "SoilHumidity_RAW:,%d,SoilHumidityPercentage:,%d,", sensorVal, percentageHumidity);
    // Serial.println(SensorReading1);        

/* BME 280 Sensor **********************************************************************/
    TEMP = bme.readTemperature();
    hPa = (bme.readPressure() / 100.0F);
    HUMID = bme.readHumidity();


    char hPaString [6];
    
    dtostrf(hPa, 6, 2, hPaString);
    sprintf(SensorReading2, "Temperature:,%d,C°,AirPressure,%s,hPa,AirHumidity:,%i ", TEMP, hPaString, HUMID);
 //   Serial.println(SensorReading2);
          
}


/****************************************************************************************/
/* BUILD DATA STRING FUNCTION ***********************************************************/
/****************************************************************************************/

void GetDataString(){
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




