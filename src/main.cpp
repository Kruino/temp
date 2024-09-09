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

// const char *ssid = "Linksys00254";
// const char *password = "35djhay7xz";

DisplayHandler Display;

int light_sensor = 36;

const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;




void setup()
{


  Serial.begin(9600);
  Wire.begin();
  M5.begin();


  Display.TopMenuVisible = true;
  Display.SetNavColorFromRgb(214, 120, 58);
  Display.SetTopBarText("Initializing");

  DataManager::Initialize();

  M5.Lcd.setCursor(0, 40);
  WifiHandler::ConnectWifi();

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  Display.BottomMenuVisible = false;
  Display.CheckNavbars();

  //bool res2 = Display.StartQuestion("Test question");


  Display.SetTopBarText("Overview");
  Display.ClearDisplay();
}

unsigned long previousMillis = 0;
const long interval = 10000;
float V_ref = 0.1;

unsigned long previousMillisMain = 10000;
unsigned long previousMillisTemp = 100000;
unsigned long previousMillisLight = 150000;

const long intervalMain = 10000;

float SPL_dB;
float cTemp;
int raw_light;
float humidity;

void loop()
{
  M5.update();

  String localTime = WifiHandler::getLocalDateTime();

  if(M5.BtnA.wasPressed()){
    Display.captureScreen();
    Display.SetTopBarText("Screenshot taken");
    delay(1000);
    Display.SetTopBarText("Overview");
  }
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillisMain >= intervalMain)
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

    cTemp = ((((data[0] * 256.0) + data[1]) * 175) / 65535.0) - 45;
    humidity = ((((data[3] * 256.0) + data[4]) * 100) / 65535.0);

    int micValue = analogRead(M5STACKFIRE_MICROPHONE_PIN);
    float voltage = micValue * (5.0 / 1023.0);
    SPL_dB = 20 * log10(voltage / V_ref);

    char charVal[8];

    dtostrf(cTemp, 4, 2, charVal);
    Display.DrawnBox(charVal, "/Images/Temp.png", 3, 34, 155, 60);
    dtostrf(humidity, 4, 2, charVal);
    Display.DrawnBox(charVal, "/Images/Humidity.png", 162, 34, 155, 60);
    dtostrf(SPL_dB, 4, 2, charVal);
    Display.DrawnBox(charVal, "/Images/Noise.png", 3, 98, 155, 60);
    dtostrf(raw_light / 4, 3, 0, charVal);
    Display.DrawnBox(charVal, "/Images/Sun.png", 162, 98, 155, 60);

  }


  if (currentMillis - previousMillisTemp >= (DataManager::TemperatureTime * 60000))
  {
    previousMillisTemp = currentMillis;

    StaticJsonDocument<200> doc;

    JsonObject object = doc.to<JsonObject>();
    object["temperature"] = (double)cTemp;
    object["locationId"] = DataManager::locationID;
    object["humidity"] = (double)humidity;

    String jsonString;
    serializeJson(object, jsonString);

    int result = Api::PostData("/temperature", jsonString);
    
    String finalres = "Temp Upload: " + localTime;
    Display.DrawnBox(finalres.c_str(), "", 3, 162, 314, 32);
  }

  if (currentMillis - previousMillisLight >= (DataManager::LightTime * 60000))
  {
    previousMillisLight = currentMillis;

    StaticJsonDocument<200> doc;

    JsonObject object = doc.to<JsonObject>();
    object["lightlevel"] = raw_light / 4;
    object["locationId"] = DataManager::locationID;

    String jsonString;
    serializeJson(object, jsonString);

    int result = Api::PostData("/lightlevel", jsonString);

    String finalres = "Light Upload: " + localTime;

    Display.DrawnBox(finalres.c_str(), "", 3, 198, 314, 32);
  }
}
