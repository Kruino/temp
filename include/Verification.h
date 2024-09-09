#ifndef VERIFICATION_H
#define VERIFICATION_H

#include <HTTPClient.h>
#include <M5Stack.h>

class Verification
{
public:
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
};


#endif