#include "DataManager.h"
#include <M5Stack.h>
#include <Api.h>
#include <CustomDisplayHandler.h>
#include <ArduinoJson.h>
#include <MQTT.h>

String DataManager::filePath = "/Data.json";
String DataManager::SSID = "";
String DataManager::password = "";
String DataManager::DeviceName = "M5Go";
String DataManager::ApiUrl = "";
String DataManager::Token = "";
int DataManager::locationID = 0;
String DataManager::location_name = "";
int DataManager::TemperatureTime = 10;
int DataManager::LightTime = 15;
JsonArray DataManager::Locations;
bool DataManager::isFarenheit = false;
String DataManager::mosquitto_server = "";
String DataManager::mosquitto_username = "";
String DataManager::mosquitto_password = "";
int DataManager::mosquitto_port = 1883;
String DataManager::MACID = String((uint64_t)ESP.getEfuseMac(), HEX);
float DataManager::cTemp = 0;
float DataManager::humidity = 0;
int DataManager::lightLevel = 0;
float DataManager::SPL_dB = 0;
String DataManager::LastLightLevelUploadTime ="";
String DataManager::LastTemperatureUploadTime = "";

String DataManager::token = "";

int DataManager::maxTemperature = 0;

uint16_t DataManager::DisplayColor = DARKGREY;

void DataManager::Initialize(){
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
            JsonDocument jsonDoc;

            jsonDoc["DeviceName"] = "M5Go";
            jsonDoc["ApiUrl"] = "";
            jsonDoc["Token"] = "";

            jsonDoc["SSID"] = "";
            jsonDoc["Password"] = "";

            jsonDoc["TemperatureDelay"] = 10;
            jsonDoc["LightDelay"] = 15;
            jsonDoc["MaxTemperature"] = 25;

            jsonDoc["LocationID"] = "";

            jsonDoc["isFahrenheit"] = false;

            jsonDoc["DisplayColor"] = std::to_string(DARKGREY);

            jsonDoc["DisplayColor"] = std::to_string(DARKGREY);

            jsonDoc["Mosquitto"]["Server"] = "";
            jsonDoc["Mosquitto"]["Username"] = "";
            jsonDoc["Mosquitto"]["Password"] = "";
            jsonDoc["Mosquitto"]["Port"] = "";

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

            while (1);
        }
}

void DataManager::Load(){
    File SetupFile = SD.open(filePath, FILE_READ);

        JsonDocument jsonDoc;
        deserializeJson(jsonDoc, SetupFile);
        SetupFile.close();

        DataManager::DeviceName = jsonDoc["DeviceName"].as<String>();

        DataManager::SSID = jsonDoc["SSID"].as<String>();
        DataManager::password = jsonDoc["Password"].as<String>();
        DataManager::ApiUrl = jsonDoc["ApiUrl"].as<String>();
        DataManager::Token = jsonDoc["Token"].as<String>();
        DataManager::locationID = jsonDoc["LocationID"].as<int>();
        DataManager::location_name = jsonDoc["LocationName"].as<String>();

        DataManager::TemperatureTime = jsonDoc["TemperatureDelay"];
        DataManager::LightTime = jsonDoc["LightDelay"];
        DataManager::isFarenheit =  jsonDoc["isFahrenheit"].as<bool>();
        DataManager::maxTemperature = jsonDoc["MaxTemperature"].as<int>();
        DataManager::mosquitto_server = jsonDoc["Mosquitto"]["Server"].as<String>();
        DataManager::mosquitto_username = jsonDoc["Mosquitto"]["Username"].as<String>();
        DataManager::mosquitto_password = jsonDoc["Mosquitto"]["Password"].as<String>();
        DataManager::mosquitto_port = jsonDoc["Mosquitto"]["Port"].as<int>();

        DataManager::DisplayColor = jsonDoc["DisplayColor"].as<uint16_t>();

}

void DataManager::Save(){
     File SetupFile = SD.open(filePath, FILE_WRITE);

        JsonDocument jsonDoc = GetJsonDocument(SetupFile);

        serializeJsonPretty(jsonDoc, SetupFile);

        SetupFile.close();
}

JsonDocument DataManager::GetJsonDocument(File file, bool addDeviceId){
        JsonDocument jsonDoc;

        if(addDeviceId){
            jsonDoc["DeviceID"] = DataManager::MACID;
        }

        jsonDoc["DeviceName"] = DataManager::DeviceName;
        jsonDoc["ApiUrl"] = DataManager::ApiUrl;
        jsonDoc["Token"] = DataManager::Token;

        jsonDoc["SSID"] = DataManager::SSID;
        jsonDoc["Password"] = DataManager::password;

        jsonDoc["LocationID"] = DataManager::locationID;
        jsonDoc["LocationName"] = DataManager::location_name;

        jsonDoc["TemperatureDelay"] = DataManager::TemperatureTime;
        jsonDoc["LightDelay"] = DataManager::LightTime;
        jsonDoc["isFahrenheit"] = DataManager::isFarenheit;
        jsonDoc["MaxTemperature"] = DataManager::maxTemperature;

        jsonDoc["Mosquitto"]["Server"] = DataManager::mosquitto_server;
        jsonDoc["Mosquitto"]["Username"] = DataManager::mosquitto_username;
        jsonDoc["Mosquitto"]["Password"] = DataManager::mosquitto_password;
        jsonDoc["Mosquitto"]["Port"] = DataManager::mosquitto_port;

        jsonDoc["DisplayColor"] = std::to_string(DataManager::DisplayColor);
    return jsonDoc;
}

void DataManager::settingsCallback(char *topic, uint8_t *payload, unsigned int length){
    String str(topic);
    String message = "";
    for (unsigned int i = 0; i < length; i++) {
        message += (char)payload[i];
    }

    JsonDocument doc;

    deserializeJson(doc, message);
    CustomDisplayHandler::showCenterMessage("Device updating");


    int newLocationID = doc["Location"].as<int>();
    if(DataManager::locationID != newLocationID){
        DataManager::SetLocationFromID(newLocationID);
        DataManager::UpdateDeviceLocationOnDatabase(DataManager::locationID);
    }

    DataManager::DeviceName = doc["Name"].as<String>();
    DataManager::LightTime = doc["LightTime"].as<int>();
    DataManager::TemperatureTime = doc["TempTime"].as<int>();
    DataManager::isFarenheit =  doc["IsFahrenheit"].as<bool>();
    DataManager::maxTemperature =  doc["MaxTemp"].as<int>();
    
    try
    {
        const char* colorStr = doc["DisplayColor"];
    
        uint16_t color = strtoul(colorStr, nullptr, 0);
        if(DataManager::DisplayColor != color){
            CustomDisplayHandler::SetNavColor(color);
        }
    }
    catch(const std::exception& e)
    {
   
    }
    



    DataManager::Save();

    sleep(2);
    CustomDisplayHandler::ClearDisplay();
    CustomDisplayHandler::DrawMainDisplay();

}
void DataManager::settingsDataCallback(char *topic, uint8_t *payload, unsigned int length){
    File SetupFile = SD.open(filePath, FILE_READ);


    JsonDocument jsonDoc;
    deserializeJson(jsonDoc, SetupFile);


    SetupFile.close();

    String jsonStr;
    serializeJsonPretty(jsonDoc, jsonStr);

    MQTT::publish(DataManager::MACID+"/Settings/Response",  jsonStr);

}

void DataManager::SetLocationFromID(int id){
    JsonDocument locationsdoc = Api::GetData("/locations");

    for (JsonObject elem : locationsdoc.as<JsonArray>()) {
        const char* name = elem["name"]; 
        const int uid = elem["id"].as<int>(); 

 
        if(uid == id){
            DataManager::locationID = uid;
            DataManager::location_name = name;
        
            DataManager::Save();
        }
    }  

    CustomDisplayHandler::SetTopBarText("Overview (" + DataManager::location_name + ")");
}

bool DataManager::SetToken() {

    String body = "{\"DeviceID\": \"" + DataManager::MACID + "\"}";

    // Send POST and get back response
    JsonDocument doc = Api::Post("/device/verify", body);

    // Check if token exists and is not null
    if (doc.containsKey("token")) {
        DataManager::token = doc["token"].as<String>();
        return true;
    }

    return false;
}

void DataManager::UpdateDeviceLocationOnDatabase(int locationID){

    JsonDocument doc;

    doc["DeviceID"] = DataManager::MACID;
    doc["LocationID"] = locationID;
    String str;

    serializeJson(doc, str);


    Api::Post("/device/location", str);
}
