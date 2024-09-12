/**
* Temperature, Humidity, Noise, and Light Level Monitoring Project
* @version 1.0
* @author Rasmus Andersen <Rasm006p@edu.sde.dk>
* 
* Device: M5GO Core ESP32
* 
* External Sensors: 
*   - ENV Sensor (connected to Red port)
*   - Grovepi Light Sensor (connected to Port B)
* 
* External Extras:
*   - SD Card (4GB) (Required)
* 
* External Libraries:
*   - M5Stack
*   - ArduinoJson
*
* Description: This project involves monitoring environmental conditions, including temperature, humidity, noise level, and light level. It also includes API access to retrieve locations and post data on temperature, humidity, and light levels.
* 
* Features:
*   - On-screen menu for selecting the device's location.
*   - Ability to switch between Celsius and Fahrenheit.
*   - Option to adjust the frequency of data postings to the API for temperature and light levels.
*/


//Libraries
#include <Wire.h>
#include <M5Stack.h>
#include <ArduinoJson.h>
#include <DisplayHandler.h>
#include <DataManager.h>
#include <Api.h>
#include <WifiHandler.h>
#include "time.h"

#define ESP_WPS_MODE WPS_TYPE_PBC
#define M5STACKFIRE_MICROPHONE_PIN 34

// SHT30 I2C address is 0x44(68)
#define Addr 0x44
#define AddrPressure 0x70

//Initialize display handler. Used to handle everything about the display.
DisplayHandler Display;


// Light sensor connected to port B on the M5GO, using data pin 36
int light_sensor = 36;

// For getting the current time in Denmark. Primarily used to get the time it sends data to the API and show it on the display.
const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;
String localTime;

// Initialize Setup function
void setup()
{
  // Initialize sensor connection and M5Go features
  Wire.begin();
  M5.begin();

  // Initialize display: show top bar, set color and text.
  Display.TopMenuVisible = true;
  Display.SetNavColorFromRgb(214, 120, 58);
  Display.SetTopBarText("Initializing");

  // Check if data.json exists; create if it doesn’t and halt execution.
  DataManager::Initialize();

  // Reset the cursor 
  M5.Lcd.setCursor(0, 40);

  // Connect the device to WiFi with settings from data.json.
  WifiHandler::ConnectWifi();

  // If location ID is unset, show menu for selecting location
  if (DataManager::locationID == 0)
  {
    Display.ShowLocations();
  }

  // Configure time based on the above NTP settings.
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  // Make the bottom navbar visible and set it to the menu type of 3 (Refresh, Menu, Capture)
  Display.BottomMenuVisible = true;
  Display.CheckNavbars();
  Display.SetMenuType(3);

  // Set top bar text to overview and current location name.
  Display.SetTopBarText("Overview (" + DataManager::location_name + ")");
  Display.ClearDisplay();
}



//milliseconds for each of the loop. 
unsigned long previousMillisMain = 10000;
unsigned long previousMillisTemp = 0;
unsigned long previousMillisLight = 0;

//interval for the screens refresh 10sec. Others are set in the DataManager.
const long intervalMain = 10000;


//Initilization of different values needed in the loop
float V_ref = 0.1;
float SPL_dB;
float cTemp;
float fTemp;
int raw_light;
float humidity;

//Upload string so it can get refreshed with the standard function and not ruin anything
String light_string = "Temp Upload: NaN";
String temp_string = "Light Upload: NaN";

void loop()
{
  //Updates the M5GO 
  M5.update();

  //Get the current time.
  localTime = WifiHandler::getLocalDateTime();

  bool refresh = false;

  //if the right buttons is pressed
  if (M5.BtnC.wasPressed())
  {
    Display.captureScreen();
    Display.SetTopBarText("Screenshot taken");
    delay(1000);
    Display.SetTopBarText("Overview");
  }

  //if the middle button is pressed
  if (M5.BtnB.wasPressed())
  {
    Display.ShowMenu();
    Display.SetMenuType(3);
    refresh = true;
    Display.SetTopBarText("Overview (" + DataManager::location_name + ")");
  }

  //if the right button is pressed
  if (M5.BtnA.wasPressed())
  {
    refresh = true;
  }


  unsigned long currentMillis = millis();

  //Temperature loop run every 10mins default. Post the data to the api.
  if (currentMillis - previousMillisTemp >= (DataManager::TemperatureTime * 60000))
  {
    previousMillisTemp = currentMillis;

    JsonDocument doc;

    JsonObject object = doc.to<JsonObject>();
    object["temperature"] = (double)cTemp;
    object["locationId"] = DataManager::locationID;
    object["humidity"] = (double)humidity;

    String jsonString;
    serializeJson(object, jsonString);

    int result = Api::PostData("/temperature", jsonString);

    temp_string = "Temp Upload: " + localTime;
  }

  //Lightlevel loop run every 15mins default. Post the data to the api.
  if (currentMillis - previousMillisLight >= (DataManager::LightTime * 60000))
  {
    previousMillisLight = currentMillis;

    JsonDocument doc;

    JsonObject object = doc.to<JsonObject>();
    object["lightlevel"] = raw_light / 4;
    object["locationId"] = DataManager::locationID;

    String jsonString;
    serializeJson(object, jsonString);

    int result = Api::PostData("/lightlevel", jsonString);

    light_string = "Light Upload: " + localTime;
  }

  //Main loop get the current data for the device and shows it on the display.
  if (currentMillis - previousMillisMain >= intervalMain || refresh)
  {
    previousMillisMain = currentMillis;

    raw_light = analogRead(light_sensor);

    Wire.beginTransmission(Addr);
    Wire.write(0x2C);
    Wire.write(0x06);
    Wire.endTransmission();

    Wire.requestFrom(Addr, 6);
    unsigned int data[6] = {0};

    if (Wire.available() == 6)
    {
      for (int i = 0; i < 6; ++i)
      {
        data[i] = Wire.read();
      }
    }
    
    //Data calculations
    cTemp = ((((data[0] * 256.0) + data[1]) * 175) / 65535.0) - 45;
    fTemp = (cTemp * 9.0 / 5.0) + 32.0;

    humidity = ((((data[3] * 256.0) + data[4]) * 100) / 65535.0);

    int micValue = analogRead(M5STACKFIRE_MICROPHONE_PIN);
    float voltage = micValue * (5.0 / 1023.0);
    SPL_dB = 20 * log10(voltage / V_ref);

    char charVal[8];

    String tempValue = String(cTemp) + " C";
    if(DataManager::isFarenheit){
      tempValue = String(fTemp) + " F";
    }

    //Draws a box on the screen with a logo if needed.
    Display.DrawnBox(tempValue, "/Images/Temp.png", 3, 34, 155, 40);
    Display.DrawnBox(String(humidity) + "%", "/Images/Humidity.png", 162, 34, 155, 40);
    Display.DrawnBox(String(SPL_dB) + "db", "/Images/Noise.png", 3, 78, 155, 40);
    Display.DrawnBox(String(raw_light / 12), "/Images/Sun.png", 162, 78, 155, 40);

    Display.DrawnBox(temp_string, "", 3, 122, 314, 32);
    Display.DrawnBox(light_string, "", 3, 158, 314, 32);
  }
}
