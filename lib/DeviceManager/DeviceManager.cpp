#include <DeviceManager.h>
#include <M5Stack.h>
#include <MQTT.h>
#include <DataManager.h>
#include <CustomDisplayHandler.h>


void DeviceManager::restartCallback(char *topic, uint8_t *payload, unsigned int length){
    CustomDisplayHandler::ClearDisplay();
    ESP.restart();
}

void DeviceManager::deviceVerifiedCallback(char *topic, uint8_t *payload, unsigned int length){

    CustomDisplayHandler::showCenterMessage("Device authorized");
    DataManager::SetToken();
    CustomDisplayHandler::ClearDisplay();
    CustomDisplayHandler::DrawMainDisplay();
}

void DeviceManager::deviceUnverifiedCallback(char *topic, uint8_t *payload, unsigned int length){
    DataManager::token = "";
    CustomDisplayHandler::showCenterMessage("Device unauthorized");
    sleep(2);
    CustomDisplayHandler::ClearDisplay();
    CustomDisplayHandler::DrawMainDisplay();
}

void DeviceManager::publishMQTTUpdate(){
 JsonDocument doc;

  doc["DeviceID"] = DataManager::MACID;
  doc["Name"] = DataManager::DeviceName;
  doc["TempTime"] = DataManager::TemperatureTime;
  doc["LightTime"] = DataManager::LightTime;
  doc["IsFahrenheit"] = DataManager::isFarenheit;
  doc["Location"] = DataManager::locationID;
  doc["MaxTemp"] =  DataManager::maxTemperature;

  char hexString[7]; 
  sprintf(hexString, "0x%04X", DataManager::DisplayColor);
  
  doc["DisplayColor"] = hexString;

  doc["data"]["temp"] = DataManager::cTemp;
  doc["data"]["Humidity"] = DataManager::humidity;
  doc["data"]["LightLevel"] = DataManager::lightLevel;

  String docString;
  serializeJson(doc, docString);


  MQTT::publish("device/Update", docString);
}

