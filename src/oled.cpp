#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

int initScreen()
{
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3c))
    {
        return -1;
    }

    display.clearDisplay();

    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(15, 20);
    display.println(F("LoraMqtt"));
    display.setCursor(20, display.getCursorY());
    display.println(F("Gateway"));

    display.display();

    return 0;
}

void refreshInfoScreen(boolean mqtt_state, IPAddress ip, uint32_t free_heap, unsigned int msg_counter)
{
    display.clearDisplay();

    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    // show banner
    display.setCursor(15,0);
    display.println(F("LoraMqtt Gateway"));
    display.println("");
    
    // show mqtt state
    display.print(F("Mqtt state: "));
    display.println(mqtt_state ? F("online") : F("offline"));

    // show ip
    display.print(F("IP: "));
    display.println(ip);

    // show free heap
    display.print(F("Free heap: "));
    display.println(free_heap);

    // show msg counter
    display.print(F("Relayed msgs: "));
    display.println(msg_counter);

    display.display();
}