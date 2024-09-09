#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include <M5Stack.h>
#include <ArduinoJson.h>
#include <DisplayHandler.h>

class DataManager
{
private:
    static String filePath;

public:
    static String ApiUrl;
    static String Token;
    static JsonArray Locations;
    static int locationID;

    static String SSID;
    static String password;
    static String locationUrl;
    static String temperatureUrl;

    static int LightTime;
    static int TemperatureTime;
    

    static void Initialize()
    {

       if (!SD.begin())
        {
            M5.Lcd.setCursor(0, 30);
            M5.Lcd.print("Missing SD Card. \n\nPlease insert the SD card \nand restart the device.");
        }
        bool exists = SD.exists(filePath);
        if (exists)
        {
            Load();
        }
        else
        {
            File SetupFile = SD.open(filePath, FILE_WRITE);

            if (!SetupFile)
            {
                Serial.println("Error opening file for writing.");
                while (1)
                    ;
            }
            DynamicJsonDocument jsonDoc(1024);

            jsonDoc["Url"] = "";
            jsonDoc["Token"] = "";

            jsonDoc["SSID"] = "";
            jsonDoc["Password"] = "";

            jsonDoc["TemperatureDelay"] = 10;
            jsonDoc["LightDelay"] = 15;

            jsonDoc["LocationID"] = 0;


            if (serializeJsonPretty(jsonDoc, SetupFile) == 0)
            {
                Serial.println("Failed to write JSON to file.");
            }
            else
            {
                Serial.println("JSON written successfully.");
            }

            SetupFile.close();

            M5.Lcd.clear();
            M5.Lcd.setCursor(0, 0);
            M5.Lcd.print("Setup file Created on SD Card. Please open the file on a computer to finish setup.");
            M5.Lcd.println("\nThe restart the device.");

            while (1)
                ;
        }
    }

    static void Load()
    {
        File SetupFile = SD.open(filePath, FILE_READ);

        JsonDocument jsonDoc;
        deserializeJson(jsonDoc, SetupFile);
        SetupFile.close();

        SSID = jsonDoc["SSID"].as<String>();
        password = jsonDoc["Password"].as<String>();
        ApiUrl = jsonDoc["Url"].as<String>();
        Token = jsonDoc["Token"].as<String>();
        locationID = jsonDoc["LocationID"].as<int>();

        TemperatureTime = jsonDoc["TemperatureDelay"];
        LightTime = jsonDoc["LightDelay"];
    }
    static void Save()
    {
        File SetupFile = SD.open(filePath, FILE_WRITE);

        JsonDocument jsonDoc;

        jsonDoc["API"]["Url"] = ApiUrl;
        jsonDoc["Token"] = Token;

        jsonDoc["SSID"] = SSID;
        jsonDoc["Password"] = password;

        jsonDoc["LocationID"] = locationID;

        jsonDoc["TemperatureDelay"] = TemperatureTime;
        jsonDoc["LightDelay"] = LightTime;

        serializeJsonPretty(jsonDoc, filePath);

        SetupFile.close();
    }
};

String DataManager::filePath = "/Data.json";
String DataManager::SSID = "";
String DataManager::password = "";
String DataManager::ApiUrl = "";
String DataManager::Token = "";
int DataManager::locationID = 0;
int DataManager::TemperatureTime = 10;
int DataManager::LightTime = 15;

#endif