#ifndef WIFIHANDLER_H
#define WIFIHANDLER_H

#include <DataManager.h>
#include <WiFi.h>
#include <time.h>

class WifiHandler
{
public:
    //Connects to wifi
    static void ConnectWifi()
    {
        WiFi.mode(WIFI_STA);
        WiFi.begin(DataManager::SSID, DataManager::password);

        CustomDisplayHandler::showCenterMessage("Connecting");

        int loopcount = 0;
        while (WiFi.status() != WL_CONNECTED)
        {
            loopcount++;

            if(loopcount > 50){
                break;
            }
            delay(250);
        }

        if(WiFi.status() != WL_CONNECTED){
            CustomDisplayHandler::ShowText("Could not connect to wifi.\nPlease check setup in file, and try again");
            while (1);
            
        }

        IPAddress myIP = WiFi.localIP();
        CustomDisplayHandler::showCenterMessage("Connected: " + myIP.toString());
        sleep(2);
    }

    //Gets the current time
    static String getLocalDateTime()
    {
        char dateL[80];

        time_t rawtime;
        struct tm *timeinfo;

        time(&rawtime);

        timeinfo = localtime(&rawtime);

        if (timeinfo == nullptr)
        {
            return "Invalid time";
        }

        strftime(dateL, sizeof(dateL), "%H:%M:%S", timeinfo);

        return String(dateL);
    }
};

#endif
