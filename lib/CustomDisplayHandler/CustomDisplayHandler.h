#ifndef CUSTOMDISPLAYHANDLER_H
#define CUSTOMDISPLAYHANDLER_H


#include <M5Stack.h>
#include <ArduinoJson.h>

class CustomDisplayHandler
{

public:
    // Defined screen dimensions
    static const int screenHeight = 240;
    static const int screenWidth = 320;

    // Height of the 2 navbars
    static const int topMenuHeight = 20;
    static const int bottomMenuHeight = 30;

    // Initialization if the initial height of the content container.
    static int maxY;
    static int startY;

    // Bool to say if the menus are currently on screen on not.
    static bool BottomMenuIsOnScreen;
    static bool TopMenuIsOnScreen;

    // For reload purposes case i have to reload the title or other values
    static int CurrentMenuType;

    // Page handling for menus.
    static int itemsPerPage;
    static int page;
    static bool first;

    // Handling the list items in the menues
    static int lastItemListNumber;
    static int lastItemNumber;

    // Offset setting for the list view
    static const int offsetX = 7;
    static const int offsetY = 0;
    static const int radius = 5;
    static const int ListStartX = 30;

    // padding for the list view
    static const int padding = 5;

    // Top bare current and last value initialization.
    static String topBarTextOld;
    static String topBarTextCurrent;


    // Values to set the visibility of the header and the navbar.
    static bool BottomMenuVisible;
    static bool TopMenuVisible;

    // Menu color.


    static void SetCursor();
    static void SetTopBarText(String text);
    static bool StartQuestion(String text, int menuType = 1);
    static void SelectItem(int itemNumber, int currentPage);
    static int FindIndex(JsonDocument doc, int target);
    static void ShowMenu(int selected = 1);
    static void ShowLocations();
    static int ShowList(int min, int max, JsonDocument list, String title = "Menu", int currentlySelected = 1);
    static void writeTextWithBgColor(String text, int posX, int posY, uint16_t color);
    static int showMinuteSelector(int initialValue, String title);
    static void OpenNavbars(int type = 1);
    static void SetMenuType(int type);
    static void AddMenuButtons(int type = 0);
    static void SetScreenSize();
    static void CheckNavbars();
    static void DrawnBox(String content, const char *iconPath, int x, int y, int w, int h);
    static void ShowText(String text);
    static void ClearDisplay();
    static void SetNavColorFromRgb(uint8_t red, uint8_t green, uint8_t blue);
    static void SetNavColor(int color);
    static void captureScreen();
    static String generateFileName();
    static void writeBMPHeader(File &file, uint16_t width, uint16_t height);
    static void showCenterMessage(String text);
    static void DrawMainDisplay();
};





#endif