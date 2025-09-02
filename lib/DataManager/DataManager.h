#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include <ArduinoJson.h>
#include <M5Stack.h>

class DataManager
{
public:
    static String filePath;

    static String token;

    static String DeviceName;
    static String ApiUrl;
    static String Token;
    static JsonArray Locations;
    static int locationID;
    static String location_name;

    static String SSID;
    static String password;
    static String locationUrl;
    static String temperatureUrl;
    static bool isFarenheit;
    
    //Mosquitto
    static String mosquitto_server;
    static String mosquitto_username;
    static String mosquitto_password;
    static int mosquitto_port;

    static int LightTime;
    static int TemperatureTime;
    
    static String MACID;


    static float cTemp;
    static float humidity;
    static int lightLevel;
    static float SPL_dB;
    static String LastTemperatureUploadTime;
    static String LastLightLevelUploadTime;

    static int DisplayColor;

    static void Initialize();
    static void Load();
    static void Save();
    static JsonDocument GetJsonDocument(File file, bool addDeviceId = false);


    static bool SetToken();
    static void SetLocationFromID(int id);
    static void settingsCallback(char *topic, uint8_t *payload, unsigned int length);
    static void settingsDataCallback(char *topic, uint8_t *payload, unsigned int length);

    static void UpdateDeviceLocationOnDatabase(int locationID);
};


#endif