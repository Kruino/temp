#ifndef API_H
#define API_H

#include <HTTPClient.h>
#include <M5Stack.h>
#include <DataManager.h>

class Api
{
public:
    static String ApiUrl;

    static bool checkURL(String url)
    {
        HTTPClient http;
        http.begin(url);

        int httpCode = http.GET();

        if (httpCode > 0)
        {
            Serial.printf("HTTP Code: %d\n", httpCode);
            http.end();
            return true;
        }
        else
        {
            Serial.printf("Error: %s\n", http.errorToString(httpCode).c_str());
            http.end();
            return false;
        }
    }

    static int PostData(String path, String body)
    {
        HTTPClient http;

        http.begin(DataManager::ApiUrl + path);
        http.addHeader("Content-Type", "application/json");
        http.addHeader("Authorization", "Bearer " + DataManager::Token);

        int httpResponseCode = http.POST(body);

        return httpResponseCode;
    }

    static JsonDocument GetData(const String &path)
    {
        HTTPClient http;
        JsonDocument doc; // Adjust size as needed


        // Construct the full URL and begin the HTTP request
        http.begin(DataManager::ApiUrl + path);
        http.addHeader("Content-Type", "application/json");
        http.addHeader("Authorization", "Bearer " + DataManager::Token);

        int httpResponseCode = http.GET();

        if (httpResponseCode == 200)
        { // Check if the request was successful
            String stream = http.getString();
           
            DeserializationError error = deserializeJson(doc, stream);

        }
        else
        {
            M5.Lcd.println("Failed to fetch data");
        }

        // End HTTP request
        http.end();

        return doc;
    }
};

#endif