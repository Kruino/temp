// #ifndef API_H
// #define API_H

// #include <HTTPClient.h>
// #include <M5Stack.h>
// #include <DataManager.h>
// #include <WiFi.h>
// #include <PubSubClient.h>


// class Api
// {



// public:


//     static int GetHttpCode(const String &path)
//     {
//         HTTPClient http;
//         JsonDocument doc; 


//         http.begin(DataManager::ApiUrl + path + "?DeviceID="+DataManager::MACID);
//         // http.addHeader("Content-Type", "application/json");


//         int httpResponseCode = http.GET();


//         return httpResponseCode;
//     }

//     //Get data from api. Mainly used to get locations from api
//     static JsonDocument GetData(const String &path)
//     {
//         HTTPClient http;
//         JsonDocument doc; 


//         // Construct the full URL and begin the HTTP request
//         http.begin(DataManager::ApiUrl + path + "?DeviceID="+DataManager::MACID);
//         http.addHeader("Content-Type", "application/json");
//         //http.addHeader("Authorization", "Bearer " + DataManager::Token);

//         int httpResponseCode = http.GET();

//         if (httpResponseCode == 200)
//         { // Check if the request was successful
//             String stream = http.getString();
           
//             DeserializationError error = deserializeJson(doc, stream);

//         }
//         else
//         {
//                M5.Lcd.println("Failed to fetch data");
//                sleep(5);
//         }

//         // End HTTP request
//         http.end();

//         // return doc;

//         return doc;
//     }
// };


// #endif
