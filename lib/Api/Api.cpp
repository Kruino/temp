#include <Api.h>
#include <HTTPClient.h>
#include <M5Stack.h>
#include <DataManager.h>
#include <WiFi.h>


int Api::GetHttpCode(const String &path)
    {
        HTTPClient http;
        JsonDocument doc; 


        http.begin(DataManager::ApiUrl + path + "?DeviceID="+DataManager::MACID);
        // http.addHeader("Content-Type", "application/json");


        int httpResponseCode = http.GET();


        return httpResponseCode;
    }

//Get data from api. Mainly used to get locations from api
JsonDocument Api::GetData(const String &path)
    {
        HTTPClient http;
        JsonDocument doc; 



        http.begin(DataManager::ApiUrl + path + "?DeviceID="+DataManager::MACID);
        http.addHeader("Content-Type", "application/json");
        http.addHeader("Authorization", "Bearer " + DataManager::token); 
        //http.addHeader("Authorization", "Bearer " + DataManager::Token);

        int httpResponseCode = http.GET();

        if (httpResponseCode == 200)
        {
            String stream = http.getString();
           
            DeserializationError error = deserializeJson(doc, stream);

        }
        else
        {
               M5.Lcd.println("Failed to fetch data");
               sleep(5);
        }

        http.end();

        // return doc;

        return doc;
    }

JsonDocument Api::Post(const String &path, String body){
    HTTPClient http;
    JsonDocument doc; 

    // Build full URL
    String url = DataManager::ApiUrl + path;

    http.begin(url);


    http.addHeader("Content-Type", "application/json");  
    http.addHeader("Accept", "application/json");        
    http.addHeader("Authorization", "Bearer " + DataManager::token); 


    int httpCode = http.POST(body);

    if (httpCode > 0) {
        String payload = http.getString();
        DeserializationError error = deserializeJson(doc, payload);

        if (error) {
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(error.f_str());
        }
    } else {
        Serial.print(F("HTTP POST failed, error: "));
        Serial.println(http.errorToString(httpCode));
    }

    http.end(); 
    return doc;
}