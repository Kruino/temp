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

const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;
String localTime;


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

  if (DataManager::locationID == 0)
  {
    Display.ShowLocations();
  }

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  Display.BottomMenuVisible = true;
  Display.CheckNavbars();
  Display.SetMenuType(3);

  Display.SetTopBarText("Overview (" + DataManager::location_name + ")");
  Display.ClearDisplay();

}

unsigned long previousMillis = 0;
const long interval = 10000;
float V_ref = 0.1;

unsigned long previousMillisMain = 10000;
unsigned long previousMillisTemp = 0;
unsigned long previousMillisLight = 0;

const long intervalMain = 10000;

float SPL_dB;
float cTemp;
float fTemp;
int raw_light;
float humidity;


String light_string = "Temp Upload: NaN";
String temp_string = "Light Upload: NaN";

void loop()
{
  M5.update();

  localTime = WifiHandler::getLocalDateTime();


  bool refresh = false;

  if (M5.BtnC.wasPressed())
  {
    Display.captureScreen();
    Display.SetTopBarText("Screenshot taken");
    delay(1000);
    Display.SetTopBarText("Overview");
  }

  if (M5.BtnB.wasPressed())
  {
    Display.ShowMenu();
    Display.SetMenuType(3);
    refresh = true;
    Display.SetTopBarText("Overview (" + DataManager::location_name + ")");
  }

  if (M5.BtnA.wasPressed())
  {
    refresh = true;
  }

  unsigned long currentMillis = millis();

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
    Display.DrawnBox(tempValue, "/Images/Temp.png", 3, 34, 155, 40);
    Display.DrawnBox(String(humidity) + "%", "/Images/Humidity.png", 162, 34, 155, 40);
    Display.DrawnBox(String(SPL_dB) + "db", "/Images/Noise.png", 3, 78, 155, 40);
    Display.DrawnBox(String(raw_light / 12), "/Images/Sun.png", 162, 78, 155, 40);

    Display.DrawnBox(temp_string, "", 3, 122, 314, 32);
    Display.DrawnBox(light_string, "", 3, 158, 314, 32);
  }
}
