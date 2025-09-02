#ifndef DEVICE_MANAGER_H
#define DEVICE_MANAGER_H

#include <M5Stack.h>

class DeviceManager
{
public:
    static void restartCallback(char *topic, uint8_t *payload, unsigned int length);
    static void deviceUnverifiedCallback(char *topic, uint8_t *payload, unsigned int length);
    static void deviceVerifiedCallback(char *topic, uint8_t *payload, unsigned int length);
    static void publishMQTTUpdate();

};


#endif