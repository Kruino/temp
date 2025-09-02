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
    - PubSubClient
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
#include <CustomDisplayHandler.h>
#include <DataManager.h>
#include <DeviceManager.h>
#include <Api.h>
#include <WifiHandler.h>
#include "time.h"
#include <MQTT.h>

#define ESP_WPS_MODE WPS_TYPE_PBC
#define M5STACKFIRE_MICROPHONE_PIN 34

// SHT30 I2C address is 0x44(68)
#define Addr 0x44
#define AddrPressure 0x70



// Light sensor connected to port B on the M5GO, using data pin 36
int light_sensor = 36;

// For getting the current time in Denmark. Primarily used to get the time it sends data to the API and show it on the display.
const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;
String localTime;


void updateCallback(char *topic, uint8_t *payload, unsigned int length){
 DeviceManager::publishMQTTUpdate();
}



// Initialize Setup function
void setup()
{
  // Initialize sensor connection and M5Go features
  Wire.begin();
  M5.begin();
  // Check if data.json exists; create if it doesn’t and halt execution.
  DataManager::Initialize();

  // Initialize display: show top bar, set color and text.
  CustomDisplayHandler::TopMenuVisible = true;
  //CustomDisplayHandler::SetNavColorFromRgb(64, 64, 64);
  CustomDisplayHandler::SetTopBarText("Initializing");



  // Reset the cursor 
  M5.Lcd.setCursor(0, 40);

  // Connect the device to WiFi with settings from data.json.
  WifiHandler::ConnectWifi();

  if(!DataManager::SetToken()){
    CustomDisplayHandler::ShowText("Device unauthorized.\n\nPlease authorize the device from the dashboard to allow uploads to database.");
    sleep(10);
  }



  MQTT::connect();
  DeviceManager::publishMQTTUpdate();


  MQTT::subscribe("device/Update/request", updateCallback);
  MQTT::subscribe(DataManager::MACID+"/Settings/Request", DataManager::settingsDataCallback);
  MQTT::subscribe(DataManager::MACID+"/Device/Restart", DeviceManager::restartCallback);
  MQTT::subscribe(DataManager::MACID+"/Device/Verified", DeviceManager::deviceVerifiedCallback);
  MQTT::subscribe(DataManager::MACID+"/Device/Unverified", DeviceManager::deviceUnverifiedCallback);
  MQTT::subscribe(DataManager::MACID+"/Settings/Update", DataManager::settingsCallback);

  

  // int authCode = Api::GetHttpCode("/verify");
  // if(authCode != 200){
  //   DisplayHandler::ShowText("Device not authorized. \n\nPlease contact the \nadministrator. \n\nDeviceID: " + DataManager::MACID + "\n\nResponse code: " + authCode);

  //   while(1);
  // }


  // Api::Setup();

  
  // If location ID is unset, show menu for selecting location
  if (DataManager::locationID == 0)
  {
    CustomDisplayHandler::ShowLocations();
  }

  // Configure time based on the above NTP settings.
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  // Make the bottom navbar visible and set it to the menu type of 3 (Refresh, Menu, Capture)
  CustomDisplayHandler::BottomMenuVisible = true;
  CustomDisplayHandler::CheckNavbars();
  CustomDisplayHandler::SetMenuType(3);

  // Set top bar text to overview and current location name.
  CustomDisplayHandler::SetTopBarText("Overview (" + DataManager::location_name + ")");
  CustomDisplayHandler::ClearDisplay();
}



//milliseconds for each of the loop. 
const unsigned long intervalRestart = 86400000;
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
  //MQTT loop
  MQTT::loop();

  //Updates the M5GO 
  M5.update();

  //Get the current time.
  localTime = WifiHandler::getLocalDateTime();

  bool refresh = false;

  //if the right buttons is pressed
  if (M5.BtnC.wasPressed())
  {
    CustomDisplayHandler::captureScreen();
    CustomDisplayHandler::SetTopBarText("Screenshot taken");
    delay(1000);
    CustomDisplayHandler::SetTopBarText("Overview");
  }

  //if the middle button is pressed
  if (M5.BtnB.wasPressed())
  {
    CustomDisplayHandler::ShowMenu();
    CustomDisplayHandler::SetMenuType(3);
    refresh = true;
    CustomDisplayHandler::SetTopBarText("Overview (" + DataManager::location_name + ")");
  }

  //if the right button is pressed
  if (M5.BtnA.wasPressed())
  {
    refresh = true;
  }


  unsigned long currentMillis = millis();

  if (currentMillis >= intervalRestart) {

    ESP.restart();
  }



  //Temperature loop run every 10mins default. Post the data to the api.
  if (currentMillis - previousMillisTemp >= (DataManager::TemperatureTime * 60000))
  {
    previousMillisTemp = currentMillis;

    DeviceManager::publishMQTTUpdate();
    DataManager::LastTemperatureUploadTime = localTime;

    JsonDocument doc;
    doc["LocationID"] = DataManager::locationID;
    doc["DeviceID"] = DataManager::MACID;
    doc["Temperature"] = DataManager::cTemp;
    doc["Humidity"] = DataManager::humidity;
    String str;
    serializeJson(doc, str);

    Api::Post("/data/temperature", str);

  }

  //Lightlevel loop run every 15mins default. Post the data to the api.
  if (currentMillis - previousMillisLight >= (DataManager::LightTime * 60000))
  {
    previousMillisLight = currentMillis;
    
    DeviceManager::publishMQTTUpdate();
    JsonDocument doc;
    doc["LocationID"] = DataManager::locationID;
    doc["DeviceID"] = DataManager::MACID;
    doc["LightLevel"] = DataManager::lightLevel;

    String str;
    serializeJson(doc, str);

    Api::Post("/data/light", str);

    DataManager::LastLightLevelUploadTime = localTime;
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

    DataManager::humidity = humidity;
    DataManager::cTemp = cTemp;
    DataManager::lightLevel = raw_light;
    DataManager::SPL_dB = SPL_dB;


    CustomDisplayHandler::DrawMainDisplay();
  }
}

