#ifndef MQTT_H
#define MQTT_H

#include <M5Stack.h>

class MQTT{
    public:
    static bool connect();
    static void loop();
    static void publish(String topic, String message);
    static void subscribe(String topic, std::function<void (char*, uint8_t*, unsigned int)>  callback);
    static void callback(char *topic, uint8_t *payload, unsigned int length);
};






#endif