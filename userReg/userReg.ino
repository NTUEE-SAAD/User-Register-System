#include <WiFi.h>
#include <HTTPClient.h>

#include <SPI.h>
#include <MFRC522.h>

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#include <NTPClient_Generic.h>
#include <WiFiUdp.h>
#include "time.h"

#include "definition.h"

const char *ssid = WIFI_SSID;       // WiFi SSID
const char *password = WIFI_PASSWD; // WiFi password

const char *GScriptId = GSCRIPT_ID;
const int httpsPort = 443;
const char *host = "https://script.google.com";

String urlPrefix = String("/macros/s/") + GScriptId + "/exec?";

// set the LCD number of columns and rows
LiquidCrystal_I2C lcd(LCD_ADDR, 20, 4);

// RFID
MFRC522 rfid(PIN_SS, PIN_RST);
MFRC522::MIFARE_Key key;
byte uid[4]; // RFID

// Network
WiFiClientSecure client;

// NTP client
const unsigned long timeOffset = 8 * 3600;
const char *ntpServer = POOL_SERVER_NAME;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, ntpServer, timeOffset);

String timeStr;
char date[8];

uint32_t lastUpdateLCD = 0;
uint32_t currentTime = 0;

void setup()
{
    Serial.begin(115200);
    SPI.begin();

    pinMode(PIN_LED_GREEN, OUTPUT);
    pinMode(PIN_LED_RED, OUTPUT);

    setupLCD();
    setupWiFi();
    setupRFID();

    timeClient.begin();

    printWelcomeLCD();
}

void loop()
{
    timeClient.update();
    // do send information
    if (detectCard())
    {
        String uidStr = "";
        for (byte i = 0; i < rfid.uid.size; i++)
        {
            String prefix = rfid.uid.uidByte[i] < 0x10 ? "0" : "";
            String hex = String(rfid.uid.uidByte[i], HEX);
            uidStr += prefix + hex;
        }
        Serial.println(uidStr);

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Waiting for");
        lcd.setCursor(0, 1);
        lcd.print("response...");

        sendData("Hi", uidStr);
        printWelcomeLCD();
    }

    currentTime = millis();
    if (currentTime - lastUpdateLCD >= 500)
    {
        snprintf(date, 8, "%02d-%02d", timeClient.getMonth(), timeClient.getDay());
        timeStr = String(timeClient.getYear()) + '-' + String(date) + ' ' + String(timeClient.getFormattedTime()) + " ";

        Serial.println(timeStr);

        lcd.setCursor(0, 0);
        lcd.print(timeStr);

        lastUpdateLCD = currentTime;
    }
}

void setupWiFi()
{
    lcd.setCursor(0, 0);
    lcd.print("Connecting to :");
    lcd.setCursor(0, 1);
    lcd.print(WIFI_SSID);
    lcd.setCursor(0, 2);

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
        lcd.print(".");
    }
    Serial.println("\nIP address: ");
    Serial.println(WiFi.localIP());

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("IP Addr:");
    lcd.setCursor(0, 1);
    lcd.print(WiFi.localIP());

    delay(1000);
    lcd.clear();
}

void sendData(String tag, String value)
{
    HTTPClient http;
    String url = String(host) + urlPrefix + String("tag=") +
                 tag + String("&value=") + value;

    // Serial.println(url);

    http.begin(url.c_str()); // Specify the URL and certificate
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    int httpCode = http.GET();

    String payload;
    if (httpCode > 0)
    { // Check for the returning code
        payload = http.getString();
        http.end();
        Serial.println(payload);

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(payload);

        digitalWrite(PIN_LED_GREEN, HIGH);

        delay(1000);

        digitalWrite(PIN_LED_GREEN, LOW);
    }
}

void setupLCD()
{
    lcd.init();
    lcd.backlight();
    // assignes each segment a write number
    // lcd.createChar(8, LT);
    // lcd.createChar(1, UB);
    // lcd.createChar(2, RT);
    // lcd.createChar(3, LL);
    // lcd.createChar(4, LB);
    // lcd.createChar(5, LR);
    // lcd.createChar(6, UMB);
    // lcd.createChar(7, LMB);
}

void printWelcomeLCD()
{
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("  NTUEE Makerspace");
    lcd.setCursor(0, 2);
    lcd.print("  User reg system");
}

void setupRFID()
{
    rfid.PCD_Init();
    rfid.PCD_DumpVersionToSerial();
}

int detectCard()
{
    int detect_type = 0;

    if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial())
        return 0;

    // Check is the PICC of Classic MIFARE type
    MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
    if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&
        piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
        piccType != MFRC522::PICC_TYPE_MIFARE_4K)
    {
        return 0;
    }

    if (rfid.uid.uidByte[0] != uid[0] ||
        rfid.uid.uidByte[1] != uid[1] ||
        rfid.uid.uidByte[2] != uid[2] ||
        rfid.uid.uidByte[3] != uid[3])
    {
        detect_type = 1;

        // Store NUID into uid array
        for (byte i = 0; i < 4; i++)
        {
            uid[i] = rfid.uid.uidByte[i];
        }
    }
    else
    {
        // Serial.println(F("Card read previously."));
        detect_type = 2;
    }
    // Halt PICC
    rfid.PICC_HaltA();
    // Stop encryption on PCD
    rfid.PCD_StopCrypto1();

    return detect_type;
}