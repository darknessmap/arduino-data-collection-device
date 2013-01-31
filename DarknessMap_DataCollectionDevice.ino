// Darkness Map Data Collection Device
// Genevieve Hoffman, 2012
// TinyGPS Library, by Mikal Hart, download here: http://arduiniana.org/libraries/tinygps/
// TSL2561 Library, by Ladyada (Limor Fried), download here: https://github.com/adafruit/TSL2561-Arduino-Library
// RTClib, by Ladyada (Limor Fried), download here: https://github.com/adafruit/RTClib
// RTClib originally developed by Jeelabs: https://github.com/jcw/rtclib

#include "Wire.h"
#include "TSL2561.h" 
#include "SD.h"
#include "SoftwareSerial.h"
#include "TinyGPS.h"
#include "RTClib.h"

const int chipSelect = 10;
RTC_Millis RTC;
File dataFile;

int sensorPin = A0;  // input pin from sensor
int lux = 0; // variable to store the value coming from sensor
TSL2561 lightSensor(TSL2561_ADDR_FLOAT); 

TinyGPS gps;
SoftwareSerial ss(3, 4);


void setup()  
{
  Serial.begin(115200);
  Serial.print("Initializing SD card...");
  ss.begin(9600);
  pinMode(chipSelect, OUTPUT);
  RTC.adjust(DateTime(__DATE__, __TIME__));

  
  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    while (1) ;
  }
  Serial.println("card initialized.");
  
  // Open up the file we're going to log to!
  dataFile = SD.open("Data5.txt", FILE_WRITE);
  if (! dataFile) {
    Serial.println("error opening datalog.txt");
    // Wait forever since we cant write data
    while (1) ;
  }  
    lightSensor.begin();
    if (lightSensor.begin()) {
    Serial.println("Found sensor");
  } else {
    Serial.println("No sensor?");
    while (1);
  }
  
  //lightSensor.setGain(TSL2561_GAIN_0X);         // set no gain (for bright situtations) 
  lightSensor.setGain(TSL2561_GAIN_16X);      // set 16x gain (for dim situations)
  //lightSensor.setTiming(TSL2561_INTEGRATIONTIME_13MS);  // shortest integration time (bright light)
 //lightSensor.setTiming(TSL2561_INTEGRATIONTIME_101MS);  // medium integration time (medium light)
 lightSensor.setTiming(TSL2561_INTEGRATIONTIME_402MS);  // longest integration time (dim light)
}

void loop()                     // run over and over again
{
  DateTime now = RTC.now();
  bool newData = false;

 for (unsigned long start = millis(); millis() - start < 1000;)
  {
    while (ss.available())
    {
      char c = ss.read();
      // Serial.write(c); // uncomment this line if you want to see the GPS data flowing
      if (gps.encode(c)) // Did a new valid sentence come in?
        newData = true;
    }
     // Read Light Sensor
     uint32_t lum = lightSensor.getFullLuminosity();
     uint16_t ir, full;
     ir = lum >> 16;
     full = lum & 0xFFFF;
     lux = lightSensor.calculateLux(full, ir);     
  }

    if (newData)
  {
    float flat, flon;
    unsigned long age;
    int year;
    byte month, day, hour, minute, second, hundredths;
    gps.f_get_position(&flat, &flon, &age);
    gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths, &age);
        char sz[32];
    sprintf(sz, "%02d/%02d/%02d, %02d:%02d:%02d,  ",
     month, day, year, hour, minute, second);
   
    Serial.print("");
    Serial.print(flon == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flon, 6);
    Serial.print(", ");
    Serial.print(flat == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flat, 6);
    Serial.print(", ");
    Serial.print(sz); //prints out GMT time
    Serial.print("unixtime: ");
    Serial.print(now.unixtime());
    Serial.print(", Lux: "); Serial.println(lux);
    
    dataFile.print("");
    dataFile.print(flon == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flon, 6);
    dataFile.print(", ");
    dataFile.print(flat == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flat, 6);
    dataFile.print(", ");
    dataFile.print(lux);
    dataFile.print(", ");
    dataFile.println(now.unixtime());
  }

  dataFile.flush(); 
  delay(500);
}
