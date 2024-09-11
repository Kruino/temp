#ifndef DISPLAYHANDLER_H
#define DISPLAYHANDLER_H
#include <M5Stack.h>
#include <ArduinoJson.h>
#include <DataManager.h>
#include <Api.h>

class DisplayHandler
{
    // Defined screen dimensions
    int screenHeight = 240;
    int screenWidth = 320;

    // Height of the 2 navbars
    int topMenuHeight = 20;
    int bottomMenuHeight = 30;

    // Initialization if the initial height of the content container.
    int maxY = screenHeight;
    int startY = 0;

    // Bool to say if the menus are currently on screen on not.
    bool BottomMenuIsOnScreen = false;
    bool TopMenuIsOnScreen = false;

    // For reload purposes case i have to reload the title or other values
    int CurrentMenuType = 1;

    // Page handling for menus.
    int itemsPerPage = 8;
    int page = 1;
    bool first = true;

    // Handling the list items in the menues
    int lastItemListNumber = 0;
    int lastItemNumber = 0;

    // Offset setting for the list view
    int offsetX = 7;
    int offsetY = 0;
    int radius = 5;
    int ListStartX = 30;

    // padding for the list view
    const int padding = 5;

    // Top bare current and last value initialization.
    String topBarTextOld = "";
    String topBarTextCurrent = "";

public:
    // Values to set the visibility of the header and the navbar.
    bool BottomMenuVisible = false;
    bool TopMenuVisible = false;

    // Menu color.
    int navColor = DARKGREY;

    void SetCursor()
    {
        M5.Lcd.setCursor(0, startY + 5);
    }
    // Sets the topbartext value and updates the header and navbar
    void SetTopBarText(String text)
    {
        CheckNavbars();
        topBarTextOld = topBarTextCurrent;
        topBarTextCurrent = text;
        OpenNavbars(CurrentMenuType);
    }

    // Displays a question on the screen for the user to answer
    bool StartQuestion(String text)
    {
        ClearDisplay();
        M5.Lcd.setCursor(0, startY + 10);
        M5.Lcd.print(text);

        OpenNavbars(2);
        while (1)
        {
            M5.update();
            if (M5.BtnA.wasPressed())
            {
                return false;
            }

            if (M5.BtnC.wasPressed())
            {
                return true;
            }
        }
    }

    // Select function not used outside of ShowList Function. Used for the visual elements of the select function of ShowList
    void SelectItem(int itemNumber, int currentPage)
    {
        M5.Lcd.setCursor(0, 40);
        int number = itemNumber - ((currentPage - 1) * itemsPerPage);
        if (lastItemNumber > 0 && lastItemNumber <= itemsPerPage)
        {
            M5.Lcd.drawRect(offsetY, ((ListStartX - 2) + (lastItemNumber - 1) * (16 + padding)), 320, 20, BLACK);
        }

        for (int i = 1; i <= itemsPerPage; i++)
        {

            if (i == number)
            {
                M5.Lcd.drawRect(offsetY, ((ListStartX - 2) + (i - 1) * (16 + padding)), 320, 20, navColor);
            }
            else
            {
                M5.Lcd.drawRect(offsetY, ((ListStartX - 2) + (i - 1) * (16 + padding)), 320, 20, BLACK);
            }

            lastItemListNumber = itemNumber;
            lastItemNumber = number;
        }
    }

    int FindIndex(JsonDocument doc, int target)
    {
        for (int i = 0; i < doc.size(); i++)
        {
            if (doc[i]["id"].as<int>() == target)
            {
                return i;
            }
        }

        return -1;
    }
    void ShowMenu(int selected = 1)
    {
        OpenNavbars();
        SetMenuType(0);
        JsonDocument SettingsDoc;
        JsonArray SettingsArray = SettingsDoc.to<JsonArray>();

        JsonObject obj1 = SettingsArray.createNestedObject();
        obj1["name"] = "Temperature Time";

        JsonObject obj2 = SettingsArray.createNestedObject();
        obj2["name"] = "Light Time";

        JsonObject obj3 = SettingsArray.createNestedObject();

        if (DataManager::isFarenheit)
        {
            obj3["name"] = "Conversion (fahrenheit)";
        }
        else
        {
            obj3["name"] = "Conversion (celsius)";
        }

        JsonObject obj4 = SettingsArray.createNestedObject();
        obj4["name"] = "Locations ("+ DataManager::location_name + ")";

        JsonObject obj5 = SettingsArray.createNestedObject();
        obj5["name"] = "Close";

        int res = ShowList(1, SettingsArray.size(), SettingsArray, "Menu", selected);

        if (res == 1)
        {
            int value = showMinuteSelector(DataManager::TemperatureTime, "Temperature Time");
            DataManager::TemperatureTime = value;
            DataManager::Save();
            ShowMenu();
        }
        else if (res == 2)
        {
            int value = showMinuteSelector(DataManager::LightTime, "Light Time");
            DataManager::LightTime = value;
            DataManager::Save();
            ShowMenu();
        }
        else if (res == 3)
        {
            if (DataManager::isFarenheit)
            {
                DataManager::isFarenheit = false;
            }
            else
            {
                DataManager::isFarenheit = true;
            }

            DataManager::Save();

            ShowMenu(3);
        }
        else if(res == 4){
            ShowLocations();
            SetMenuType(3);
            ShowMenu(4);
        }
        else
        {
            return;
        }
    }
    // Shows a Json array on screen min will almost always be 1, And max will almost always be the size of the given array you want to show. Title is the text the top bar needs to show.

    void ShowLocations()
    {
        SetMenuType(0);
        JsonDocument locationsdoc = Api::GetData("/location");

        int index = FindIndex(locationsdoc, DataManager::locationID) + 1;

        int res = ShowList(1, locationsdoc.size(), locationsdoc, "Locations", index) - 1;

        if (locationsdoc[res]["id"].as<int>() != DataManager::locationID)
        {
            DataManager::locationID = locationsdoc[res]["id"].as<int>();
            DataManager::location_name = locationsdoc[res]["name"].as<String>();
            DataManager::Save();
        }
    }

    int ShowList(int min, int max, JsonDocument list, String title = "Menu", int currentlySelected = 1)
    {

        int totalPages = (max + itemsPerPage - 1) / itemsPerPage;

        int interaction = false;
        int changedTopMenu = false;

        // Initialize pagination
        int current = min;
        int currentPage = 1;
        bool first = true;

        current = currentlySelected;
        currentPage = ((current - 1) / itemsPerPage) + 1;
        SelectItem(currentlySelected, currentPage);

        if (totalPages != 1)
        {
            SetTopBarText(title + "    Page: " + String(currentPage) + " of " + String(totalPages));
        }
        else
        {
            SetTopBarText(title);
        }

        delay(100);

        while (true)
        {
            M5.update();

            // Update display if needed
            if (page != currentPage || first)
            {
                int firstItemIndex = (currentPage - 1) * itemsPerPage;

                ClearDisplay();

                M5.Lcd.setCursor(0, 40);

                for (int i = firstItemIndex; i < firstItemIndex + itemsPerPage && i < list.size(); i++)
                {
                    int number = i - ((currentPage - 1) * itemsPerPage);
                    String name = list[i]["name"];
                    int yPosition = ListStartX + number * (16 + padding);

                    M5.Lcd.setCursor(offsetY, yPosition);
                    M5.Lcd.printf("  %s", name.c_str());
                }
                page = currentPage;
                first = false;
                SelectItem(current, currentPage);
            }

            // Navigate up
            if (M5.BtnA.isPressed() && current > min)
            {
                current--;
                M5.Lcd.setCursor(0, 170);

                if (current < (currentPage - 1) * itemsPerPage + 1)
                {
                    currentPage--;
                    SetTopBarText(title + "    Page: " + String(currentPage) + " of " + String(totalPages));
                }
                else
                {
                    SelectItem(current, currentPage);
                }

                delay(200);
            }

            // Navigate down
            if (M5.BtnC.isPressed() && current < max)
            {
                current++;
                M5.Lcd.setCursor(0, 170);

                if (current > currentPage * itemsPerPage)
                {
                    currentPage++;
                    SetTopBarText(title + "    Page: " + String(currentPage) + " of " + String(totalPages));
                }
                else
                {
                    SelectItem(current, currentPage);
                }

                delay(200);
            }

            // Confirm selection or exit
            if (M5.BtnB.wasPressed())
            {
                CheckNavbars();
                ClearDisplay();
                return current;
            }
        }
    }

    // Primary usecase is for writing text on header and navbar. Writes text with the given color as background color so it matches the box it is inside.
    void writeTextWithBgColor(String text, int posX, int posY, uint16_t color)
    {
        M5.Lcd.setTextColor(WHITE, color);
        M5.Lcd.setCursor(posX, posY);
        M5.Lcd.println(text);
        M5.Lcd.setTextColor(WHITE, BLACK);
    }

    int showMinuteSelector(int initialValue, String title)
    {
        SetTopBarText(title);
        ClearDisplay();
        String value = "- " + String(initialValue) + "min +";
        ;
        int length = M5.Lcd.textWidth(value);
        M5.Lcd.setCursor(160 - (length / 2), 100);
        M5.Lcd.print(String(value));

        while (true)
        {
            M5.update();

            if (M5.BtnA.isPressed())
            {
                if (initialValue > 1)
                {
                    initialValue--;
                    String value = "- " + String(initialValue) + "min +";
                    int length = M5.Lcd.textWidth(value);
                    M5.Lcd.setCursor(160 - (length / 2), 100);
                    M5.Lcd.print(value);
                    delay(200);
                }
            }

            if (M5.BtnB.wasPressed())
            {
                return initialValue;
            }

            if (M5.BtnC.isPressed())
            {
                initialValue++;
                String value = "- " + String(initialValue) + "min +";
                int length = M5.Lcd.textWidth(value);
                M5.Lcd.setCursor(160 - (length / 2), 100);
                M5.Lcd.print(value);
                delay(200);
            }
        }
        SetTopBarText(topBarTextOld);
    }

    // Opens the menus selected with the TopMenuVisible and BottomMenuVisible bool. A bool is parsed to know if it needs to show Yes/no or the select options on the bottom bar.
    void OpenNavbars(int type = 1)
    {
        M5.Lcd.setTextSize(2);

        TopMenuIsOnScreen = TopMenuVisible;
        BottomMenuIsOnScreen = BottomMenuVisible;

        CurrentMenuType = type;
        if (TopMenuVisible)
        {
            M5.Lcd.fillRect(0, 0, screenWidth, topMenuHeight, navColor);
        }

        if (BottomMenuVisible)
        {
            M5.Lcd.fillRect(0, screenHeight - bottomMenuHeight, screenWidth, bottomMenuHeight, navColor);
        }

        if (TopMenuIsOnScreen)
        {
            writeTextWithBgColor(topBarTextCurrent, 5, 2, navColor);
        }

        AddMenuButtons(CurrentMenuType);
    }

    void SetMenuType(int type)
    {
        CurrentMenuType = type;
        if (BottomMenuVisible)
        {
            M5.Lcd.fillRect(0, screenHeight - bottomMenuHeight, screenWidth, bottomMenuHeight, navColor);
        }
        AddMenuButtons(CurrentMenuType);
    }

    void AddMenuButtons(int type = 0)
    {
        if (BottomMenuIsOnScreen)
        {
            if (type == 1)
            {
                writeTextWithBgColor("No", 45, 220, navColor);
                writeTextWithBgColor("Yes", 232, 220, navColor);
            }
            else if (type == 2)
            {
                writeTextWithBgColor("Menu", 115, 220, navColor);
            }
            else if (type == 3)
            {
                writeTextWithBgColor("Refresh", 25, 220, navColor);
                writeTextWithBgColor("Menu", 140, 220, navColor);
                writeTextWithBgColor("Capture", 212, 220, navColor);
            }
            else
            {
                writeTextWithBgColor("Last", 45, 220, navColor);
                writeTextWithBgColor("Continue", 115, 220, navColor);
                writeTextWithBgColor("Next", 232, 220, navColor);
            }
        }
    }

    // Calculates the current allowed height of the inner content. by screen height - topmenu height if visible and any other navbars.
    void SetScreenSize()
    {
        startY = 0;
        maxY = screenHeight;

        if (TopMenuVisible)
        {
            startY = topMenuHeight;
            maxY -= topMenuHeight;
        }

        if (BottomMenuVisible)
        {
            maxY -= bottomMenuHeight;
        }
    }

    // Checks if a header or navbar is on screen when it should not be and runs the opennavbar function to update them to their actual state.
    void CheckNavbars()
    {
        if (BottomMenuVisible != BottomMenuIsOnScreen || TopMenuVisible != TopMenuIsOnScreen)
        {
            OpenNavbars(CurrentMenuType);
        }
    }

    void DrawnBox(String content, const char *iconPath, int x, int y, int w, int h)
    {
        M5.Lcd.drawRect(x, y, w, h, navColor);
        M5.Lcd.drawRect(x + 1, y + 1, w - 2, h - 2, navColor);

        int offset = 7;
        if (iconPath != nullptr && iconPath[0] != '\0')
        {
            M5.Lcd.drawPngFile(SD, iconPath, x + 10, (y + (h / 2)) - 12, 24, 24);
            offset = 45;
        }

        int width = M5.Lcd.textWidth(content);
        M5.Lcd.setCursor(x + offset, (y + (h / 2)) - 5);

        M5.Lcd.fillRect(x + offset - 2, y + 2, w - offset, h - 4, BLACK);
        M5.Lcd.println(content);
    }

    // Shows text on the screen.
    void ShowText(String text)
    {
        SetScreenSize();
        ClearDisplay();
        M5.Lcd.setCursor(0, startY + 10);
        M5.Lcd.print(text);
    }

    // Clears the content container.
    void ClearDisplay()
    {
        CheckNavbars();
        SetScreenSize();
        M5.Lcd.fillRect(0, startY, screenWidth, maxY, BLACK);
    }

    // Tries it best to set a color the display can show from an rgb color.
    void SetNavColorFromRgb(uint8_t red, uint8_t green, uint8_t blue)
    {
        int red5 = (red * 31) / 255;
        int green6 = (green * 63) / 255;
        int blue5 = (blue * 31) / 255;

        int colorCode = (red5 << 11) | (green6 << 5) | blue5;

        navColor = colorCode;

        OpenNavbars(CurrentMenuType);
    }

    void SetNavColor(int color)
    {
        navColor = color;

        OpenNavbars(CurrentMenuType);
    }

    void captureScreen()
    {
        uint16_t width = 320;
        uint16_t height = 240;
        uint16_t *lineBuffer = (uint16_t *)malloc(width * sizeof(uint16_t));

        String fileName = generateFileName();
        File file = SD.open(fileName, FILE_WRITE);
        if (!file)
        {
            Serial.println("Failed to create file: " + fileName);
            return;
        }

        writeBMPHeader(file, width, height);

        for (uint16_t y = height; y > 0; y--)
        { // BMP files are bottom to top
            M5.Lcd.readRect(0, y - 1, width, 1, lineBuffer);
            file.write((uint8_t *)lineBuffer, width * 2);
        }

        file.close();
        free(lineBuffer);
        Serial.println("Screenshot saved: " + fileName);
    }

    String generateFileName()
    {
        int fileIndex = 1;
        String fileName;
        do
        {
            fileName = "/screenshot" + String(fileIndex) + ".bmp";
            fileIndex++;
        } while (SD.exists(fileName));
        return fileName;
    }

    static void writeBMPHeader(File &file, uint16_t width, uint16_t height)
    {
        int fileSize = 54 + 2 * width * height; // Header size (54) + RGB565 data size
        int reserved = 0;
        int dataOffset = 54;

        int headerSize = 40;
        int planes = 1;
        int bitCount = 16;
        int compression = 3; // BI_BITFIELDS for 16-bit
        int imageSize = 2 * width * height;

        // Write BMP file header
        uint8_t bmpFileHeader[14] = {
            'B', 'M',                                                                                                   // Signature
            (uint8_t)(fileSize), (uint8_t)(fileSize >> 8), (uint8_t)(fileSize >> 16), (uint8_t)(fileSize >> 24),        // File size
            0, 0,                                                                                                       // Reserved
            0, 0,                                                                                                       // Reserved
            (uint8_t)(dataOffset), (uint8_t)(dataOffset >> 8), (uint8_t)(dataOffset >> 16), (uint8_t)(dataOffset >> 24) // Data offset
        };
        file.write(bmpFileHeader, sizeof(bmpFileHeader));

        // Write BMP info header
        uint8_t bmpInfoHeader[40] = {
            (uint8_t)(headerSize), (uint8_t)(headerSize >> 8), (uint8_t)(headerSize >> 16), (uint8_t)(headerSize >> 24),     // Header size
            (uint8_t)(width), (uint8_t)(width >> 8), (uint8_t)(width >> 16), (uint8_t)(width >> 24),                         // Width
            (uint8_t)(height), (uint8_t)(height >> 8), (uint8_t)(height >> 16), (uint8_t)(height >> 24),                     // Height
            (uint8_t)(planes), (uint8_t)(planes >> 8),                                                                       // Planes
            (uint8_t)(bitCount), (uint8_t)(bitCount >> 8),                                                                   // Bit count
            (uint8_t)(compression), (uint8_t)(compression >> 8), (uint8_t)(compression >> 16), (uint8_t)(compression >> 24), // Compression
            (uint8_t)(imageSize), (uint8_t)(imageSize >> 8), (uint8_t)(imageSize >> 16), (uint8_t)(imageSize >> 24),         // Image size
            0, 0, 0, 0,                                                                                                      // X pixels per meter (not specified)
            0, 0, 0, 0,                                                                                                      // Y pixels per meter (not specified)
            0, 0, 0, 0,                                                                                                      // Total colors (not specified)
            0, 0, 0, 0                                                                                                       // Important colors (not specified)
        };
        file.write(bmpInfoHeader, sizeof(bmpInfoHeader));

        // Write RGB masks for 16-bit (RGB565)
        int redMask = 0xF800;
        int greenMask = 0x07E0;
        int blueMask = 0x001F;
        file.write((uint8_t *)&redMask, 4);
        file.write((uint8_t *)&greenMask, 4);
        file.write((uint8_t *)&blueMask, 4);

        // Padding to align the header to a 54-byte total size
        int padding = 0;
        file.write((uint8_t *)&padding, 4); // Remaining mask bits
    }
};
#endif