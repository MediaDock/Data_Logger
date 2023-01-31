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

                Hardware used: Seeeduino xiao & DS3231 Module                
                Built for a Workshop at MediaDock HSLU D&K
                Based on the following Examples: 
                https://rydepier.wordpress.com/2015/08/07/using-an-sd-card-reader-to-store-and-retrieve-data-with-arduino/
*/

int MakeABreak = 5000; // write every ... Milliseconds 

/* Realtime Clock Globals **************************************************************/
#include <RTClib.h> // tested with 2.1.0 
RTC_DS3231 rtc;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};


// To set the realtimeclock on to the correct Date and Time run the Examples > DS3231 > DS3231_set.ino first  



/* SD Card Globals **********************************************************************/
// SD Card max Size: 16GB ---> FAT16 or FAT32 Formatted 

#include <SD.h>
#include"SPI.h"
File myFile;
char filename[16]; // make it long enough to hold your longest file name, plus a null terminator
const int chipSelect = 3;
String DataString =""; // holds the data to be written to the SD card



/* Your Sensor Globals ******************************************************************/
// YOUR CODE HERE: Your Sensor may need some specific libraries include them here 
int SensorReading1 = 0;       // Pseudo Code: Replace with your Senor Data example in case it`s integer
float SensorReading2 = 0.00;  // Pseudo Code: Replace with your Senor Data example in case it`s float




/****************************************************************************************/
/* SETUP ********************************************************************************/
/****************************************************************************************/

void setup () {
    Serial.begin(57600);
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
    Serial.println("Setup Completed!");


  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1) delay(10);
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, let's set the time!");
    // When time needs to be set on a new device, or after a power loss, the
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  //  This line sets the RTC with an explicit date & time, for example to set
    //January 21, 2014 at 3am you would call:
   // rtc.adjust(DateTime(2023, 1, 27, 17, 0, 0));
  }




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
  // YOUR CODE HERE: Do Something to read your Sensor Data in this Function
  // IMPORTANT: Save your Data as a Variable called SensorReading1 or SensorReading2 
  // This SensorReadingX Variables will be passed to the GetDataString Function for further Processing
}


/****************************************************************************************/
/* BUILD DATA STRING FUNCTION ***********************************************************/
/****************************************************************************************/

void GetDataString(){
    DateTime now = rtc.now();
    DataString = String(now.day()) +      // Day Value from RTC
                 "/" +                    // Character to Format Date a bit nicer
                 String(now.month()) +    
                 "/" + 
                 String(now.year()) + 
                 "," +                    // Commas are used in CSV to indicate a new column
                 String(now.hour()) +
                 ":" + 
                 String(now.minute()) +
                 ":" + 
                 String(now.second()) + 
                 "," + 
                 "RTC_Temp:" +
                 "," + 
                 String(rtc.getTemperature()) +
                 "," + 
                 String(SensorReading1); + 
                 "," + 
                 String(SensorReading2);  // Add more if you have more Sensors to be logged
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
        Serial.println("new Line of Data saved to SD Card");
      }
    }
    else{
        Serial.println("Error writing to file !");
    }
}


