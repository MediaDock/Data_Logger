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

                Based on: Seeeduino xiao
                Built for a Workshop at MediaDock HSLU 
*/


int MakeABreak = 2000; // write every ... Milliseconds 

/* Realtime Clock Globals **************************************************************/
#include <Wire.h>
#include <DS3231.h>
RTClib myRTC;

// To set the realtimeclock on to the correct Date and Time run the Examples > DS3231 > DS3231_set.ino first  

/* SD Card Globals **********************************************************************/
// SD Card max Size: 16GB ---> FAT16 or FAT32 Formatted 

#include <SD.h>
#include"SPI.h"
File myFile;
char filename[16]; // make it long enough to hold your longest file name, plus a null terminator
const int chipSelect = 3;
unsigned long gpxSize = 0;
String DataString =""; // holds the data to be written to the SD card



/* Your Sensor Globals ******************************************************************/
int SensorReading1 = 0;       // Pseudo Code: Replace with your Senor Data example in case it`s integer
float SensorReading2 = 0.00;  // Pseudo Code: Replace with your Senor Data example in case it`s float




/****************************************************************************************/
/* SETUP ********************************************************************************/
/****************************************************************************************/

void setup () {
    Serial.begin(57600);


  /**SD Card Module Init *******************************************/
   
    Wire.begin();
    delay(500);
    Serial.print("Initializing SD card..."); 
    if (!SD.begin(chipSelect)) {
      Serial.println("SD Card initialization failed!");
      delay(2000);
      return; // don't do anything more: THE TRACKER ONLY WORKS IF AN SD CARD IS DETECTED IN THE SDCARD READER
    }
    Serial.println("SD card initialized.");


  /**File Initialization*******************************************/

    int n = 0;
    snprintf(filename, sizeof(filename), "Track%03d.csv", n); // includes a three-digit sequence number in the file name
    while (SD.exists(filename)) {
      n++;
      delay(100);
      snprintf(filename, sizeof(filename), "Track%03d.csv", n);
    }

    File myFile = SD.open(filename, FILE_WRITE);

    if (myFile) {
      myFile.close(); // close the file
    }

    Serial.print("new File initialized: ");
    Serial.println(F(filename));
    Serial.println("Setup Completed!");
}


/****************************************************************************************/
/* LOOP ********************************************************************************/
/****************************************************************************************/


void loop () {
    delay(MakeABreak);   

    GetSensorData();
    GetDataString();
    SaveData();
}



/****************************************************************************************/
/* GET DATA FROM SENSOR FUNCTION ********************************************************/
/****************************************************************************************/

void GetSensorData(){
  // YOUR CODE HERE: Do Something to read your Sensor Data
}


/****************************************************************************************/
/* BUILD DATA STRING FUNCTION ***********************************************************/
/****************************************************************************************/

void GetDataString(){
    DateTime now = myRTC.now();
    DataString = String(now.day()) +      // Day Value from RTC
                 "." +                    // Character to Format Date a bit nicer
                 String(now.month()) +    
                 "." + 
                 String(now.year()) + 
                 "," +                    // Commas are used in CSV to indicate a new row
                 String(now.hour()) +
                 ":" + 
                 String(now.minute()) +
                 ":" + 
                 String(now.second()) + 
                 "," + 
                 String(SensorReading1); + 
                 "," + 
                 String(SensorReading2);  // Add more if you have more Sensors to be logged
                 
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
      }
    }
    else{
        Serial.println("Error writing to file !");
    }
}


