# Environmental Monitoring Project

**Version:** 1.0  
**Author:** Rasmus Andersen

## Overview

 This project involves monitoring environmental conditions, including temperature, humidity, noise level, and light level. It also includes API access to retrieve locations and post data on temperature, humidity, and light levels.

## Features

- **On-screen Menu:** Allows selection of the device's location.
- **Temperature Units:** Option to switch between Celsius and Fahrenheit.
- **API Integration:** Posts temperature, humidity, and light level data to an API at configurable intervals.
- **Configurable Data Posting:** Users can adjust the frequency of data postings for temperature and light levels.

## Hardware Requirements

- **Device:** M5GO Core ESP32
- **External Sensors:**
  - **ENV Sensor:** Connected to the Red port.
  - **Grovepi Light Sensor:** Connected to Port B.
- **External Extras:**
  - **SD Card (4GB):** Required for storing configuration.

## Software Requirements

- **Libraries:**
  - `M5Stack`
  - `ArduinoJson`

## Setup and Configuration

1. **Connect the Sensors:**
   - ENV. III Sensor to the Red port.
   - Grovepi Light Sensor to Port B.

2. **Install Required Libraries:**
   - Install `M5Stack` and `ArduinoJson`

3. **Run the device:**
   - Insert a Fat32 formatted `SD Card` and `Run the device` this wil create the Data.json file
   - Turn off the device and plug the `SD Card` into a computer
   - Open `Data.json` on the `SD Card`
   - Change value `SSID`, `password` to you routers login
   - Change `ApiUrl` to your api's url.
   - Eject the `SD Card` and insert it back into the device

4. **Configure WiFi:**
   - Ensure your WiFi credentials are stored in the `Data.json` file on the SD card.
   - The device will automatically connect to WiFi using these settings.

5. **API Configuration:**
   - If custom endpoints or data are needed to be able to post to your api change  `Api.h` and posts under `main.cpp`.

6. **Location Setup:**
   - If the location ID is not set, the device will prompt you to select a location from the on-screen menu.

7. **Icons:**
   - the icons will not show up as standard. you need to open the `SD Card` on a pc.
   - Create a folder called `images`.
   - and add png icons in the folder with these name  `Temp.png`, `Humidity.png`, `Noise.png` and `Sun.png` for light.


## How It Works

- The device continuously monitors temperature, humidity, noise level, and light level.
- Data is displayed on the device screen and posted to a configured API endpoint at regular intervals.
- The user can interact with the device through the on-screen menu to adjust settings and capture screenshots.

