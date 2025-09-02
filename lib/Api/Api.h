#ifndef API_H
#define API_H

#include <M5Stack.h>
#include <ArduinoJson.h>

class Api
{
public:
    static int GetHttpCode(const String &path);
    static JsonDocument GetData(const String &path);
    static JsonDocument Post(const String &path, String body);
};


#endif