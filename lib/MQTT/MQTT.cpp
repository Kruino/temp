#include <MQTT.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <DataManager.h>
#include <ArduinoJson.h>
#include <map> //GNU ISO C++ Library. Used for dictionary

static WiFiClient espClient;
static PubSubClient client(espClient);
std::map<String, std::function<void (char*, uint8_t*, unsigned int)>> callbacks;


bool MQTT::connect() {

    if (!client.connected()) {
        client.setServer(DataManager::mosquitto_server.c_str(), DataManager::mosquitto_port);
        client.setCallback(MQTT::callback);

        while (!client.connected()) {

            if (client.connect(DataManager::MACID.c_str(), DataManager::mosquitto_username.c_str(), DataManager::mosquitto_password.c_str())) {
                    
            } else {
                delay(2000);
            }
        }
    }



    return client.connected();
}

void MQTT::loop(){
    MQTT::connect();
    client.loop(); 
}

void MQTT::publish(String topic, String message){
    client.publish(topic.c_str(), message.c_str());
}

void MQTT::subscribe(String topic, std::function<void (char*, uint8_t*, unsigned int)>  callback){
    client.subscribe(topic.c_str());
    callbacks[topic] = callback;
}

void MQTT::callback(char *topic, uint8_t *payload, unsigned int length){
    std::function<void (char*, uint8_t*, unsigned int)> callback = callbacks[topic];
    if(callback != NULL){
        callback(topic, payload, length);
    }
}
