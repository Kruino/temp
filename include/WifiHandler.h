#ifndef WIFIHANDLER_H
#define WIFIHANDLER_H

#include <DataManager.h>
#include <WiFi.h>
#include <time.h>

class WifiHandler
{
public:
    static void ConnectWifi()
    {
        WiFi.mode(WIFI_STA);
        WiFi.begin(DataManager::SSID, DataManager::password);
        M5.Lcd.print("Connecting");

        while (WiFi.status() != WL_CONNECTED)
        {
            M5.Lcd.print(".");
            delay(250);
        }

        if (WiFi.status() != WL_CONNECTED)
        {
            M5.Lcd.println("Could not connect to wifi.");
            M5.Lcd.println("Please check setup in file");

            while (1)
                ;
        }

        IPAddress myIP = WiFi.localIP();
        M5.Lcd.print("\nConnected: ");
        M5.Lcd.print(myIP);
        M5.Lcd.print("\n");
    }

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