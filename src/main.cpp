#include "Adafruit_MCP9601.h"
#include "WiFi.h"
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <Adafruit_NeoPixel.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include "./wifi-creds.h"

// wifi stuff

extern const uint8_t root_ca_pem_start[] asm("_binary_src_root_ca_pem_start");
extern const uint8_t root_ca_pem_end[] asm("_binary_src_root_ca_pem_end");

// Not sure if NetworkClientSecure checks the validity date of the certificate.
// Setting clock just to be sure...
void setClock()
{
  configTime(0, 0, "pool.ntp.org");

  Serial.print(F("Waiting for NTP time sync: "));
  time_t nowSecs = time(nullptr);
  while (nowSecs < 8 * 3600 * 2)
  {
    delay(500);
    Serial.print(F("."));
    yield();
    nowSecs = time(nullptr);
  }

  Serial.println();
  struct tm timeinfo;
  gmtime_r(&nowSecs, &timeinfo);
  Serial.print(F("Current time: "));
  Serial.print(asctime(&timeinfo));
}

WiFiMulti wifiMulti;

void connectWIFI()
{
  // get creds, split by newline and assign to ssid and password

  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);
  Serial.print("Password: ");
  Serial.println(WIFI_PASS);

  delay(1000);

  // WiFi.mode(WIFI_STA);

  wifiMulti.addAP(WIFI_SSID, WIFI_PASS);

  // wait for WiFi connection
  Serial.print("Waiting for WiFi to connect...");
  while ((wifiMulti.run() != WL_CONNECTED))
  {
    Serial.print(".");
  }
  Serial.println(" connected");

  setClock();
}

bool postTemperature(float temp)
{
  if ((wifiMulti.run() == WL_CONNECTED))
  {

    HTTPClient http;

    Serial.print("[HTTP] begin...\n");
    char *ca = (char *)root_ca_pem_start;
    http.begin("https://www.howcoldismy.beer/api/temp", ca); // HTTPS

    Serial.print("[HTTP] GET...\n");
    http.addHeader("Content-Type", "application/json");
    http.addHeader("x-api-key", API_KEY);

    String payload = "{\"temp\":\"";
    payload += temp;
    payload += "\"}";

    int httpCode = http.POST(payload);

    // httpCode will be negative on error
    if (httpCode > 0)
    {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] GET... code: %d\n", httpCode);

      // file found at server
      if (httpCode == HTTP_CODE_OK)
      {
        String payload = http.getString();
        Serial.println(payload);
      }
    }
    else
    {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
  }
  return false;
}

// Temp senseor stuff

#define TEMP_SENSOR_I2C_ADDRESS (0x67)

Adafruit_MCP9601 mcp;

float readTemperature()
{
  uint8_t status = mcp.getStatus();
  Serial.print("MCP Status: 0x");
  Serial.print(status, HEX);
  Serial.print(": ");
  float temp = 0.0;
  if (status & MCP9601_STATUS_OPENCIRCUIT)
  {
    Serial.println("Thermocouple open!");
    return temp; // don't continue, since there's no thermocouple
  }
  if (status & MCP9601_STATUS_SHORTCIRCUIT)
  {
    Serial.println("Thermocouple shorted to ground!");
    return temp; // don't continue, since the sensor is not working
  }
  if (status & MCP960X_STATUS_ALERT1)
  {
    Serial.print("Alert 1, ");
  }
  if (status & MCP960X_STATUS_ALERT2)
  {
    Serial.print("Alert 2, ");
  }
  if (status & MCP960X_STATUS_ALERT3)
  {
    Serial.print("Alert 3, ");
  }
  if (status & MCP960X_STATUS_ALERT4)
  {
    Serial.print("Alert 4, ");
  }
  Serial.println();

  temp = mcp.readThermocouple();
  Serial.print("Hot Junction: ");
  Serial.println(temp);
  Serial.print("Cold Junction: ");
  Serial.println(mcp.readAmbient());
  Serial.print("ADC: ");
  Serial.print(mcp.readADC() * 2);
  Serial.println(" uV");

  return temp;
}

// NEOPixel stuff

#define NUMPIXELS 1

Adafruit_NeoPixel pixels(NUMPIXELS, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);

void initLED()
{

#if defined(NEOPIXEL_POWER)
  // If this board has a power control pin, we must set it to output and high
  // in order to enable the NeoPixels. We put this in an #if defined so it can
  // be reused for other boards without compilation errors
  pinMode(NEOPIXEL_POWER, OUTPUT);
  digitalWrite(NEOPIXEL_POWER, HIGH);
#endif

  pixels.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  pixels.setBrightness(20); // not so bright
}

void setLED(uint8_t r, uint8_t g, uint8_t b)
{
  pixels.setPixelColor(0, pixels.Color(r, g, b));
  pixels.show();
}

void blueLED()
{
  setLED(0, 0, 255);
}

void greenLED()
{
  setLED(0, 255, 0);
}

void redLED()
{
  setLED(255, 0, 0);
}

void offLED()
{
  setLED(0, 0, 0);
}

void magentaLED()
{
  setLED(255, 0, 255);
}

void cyanLED()
{
  setLED(0, 255, 255);
}

// oled stuff

Adafruit_SH1107 display = Adafruit_SH1107(64, 128, &Wire1);

// // OLED FeatherWing buttons map to different pins depending on board:
// #if defined(ESP8266)
//   #define BUTTON_A  0
//   #define BUTTON_B 16
//   #define BUTTON_C  2
// #elif defined(ESP32) && !defined(ARDUINO_ADAFRUIT_FEATHER_ESP32S2)
//   #define BUTTON_A 15
//   #define BUTTON_B 32
//   #define BUTTON_C 14
// #elif defined(ARDUINO_STM32_FEATHER)
//   #define BUTTON_A PA15
//   #define BUTTON_B PC7
//   #define BUTTON_C PC5
// #elif defined(TEENSYDUINO)
//   #define BUTTON_A  4
//   #define BUTTON_B  3
//   #define BUTTON_C  8
// #elif defined(ARDUINO_NRF52832_FEATHER)
//   #define BUTTON_A 31
//   #define BUTTON_B 30
//   #define BUTTON_C 27
// #else // 32u4, M0, M4, nrf52840, esp32-s2 and 328p
//   #define BUTTON_A  9
//   #define BUTTON_B  6
//   #define BUTTON_C  5
// #endif

void setup()
{
  Serial.begin(115200);
  delay(1000);

  initLED();
  cyanLED();

  Serial.println("Starting WiFi...");

  delay(1000);

  // WiFi.mode(WIFI_STA);
  // WiFi.disconnect();
  connectWIFI();

  magentaLED();

  Serial.println("128x64 OLED FeatherWing test");
  delay(250);                // wait for the OLED to power up
  display.begin(0x3C, true); // Address 0x3C default

  Serial.println("OLED begun");

  // Show image buffer on the display hardware.
  // Since the buffer is intialized with an Adafruit splashscreen
  // internally, this will display the splashscreen.
  display.display();
  delay(1000);

  // Clear the buffer.
  display.clearDisplay();
  display.display();

  display.setRotation(1);

  // pinMode(BUTTON_A, INPUT_PULLUP);
  // pinMode(BUTTON_B, INPUT_PULLUP);
  // pinMode(BUTTON_C, INPUT_PULLUP);

  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0, 0);
  display.println("ayyyyyy");
  display.display(); // actually display all of the above

  /* Initialise the driver with TEMP_SENSOR_I2C_ADDRESS and the default I2C bus. */
  if (!mcp.begin(TEMP_SENSOR_I2C_ADDRESS, &Wire1))
  {
    Serial.println("Sensor not found. Check wiring!");
    redLED();
    while (1)
      ;
  }
  else
  {
    Serial.println("Found MCP9601!");
  }

  // mcp.setADCresolution(MCP9600_ADCRESOLUTION_18);
  // Serial.print("ADC resolution set to ");
  // switch (mcp.getADCresolution()) {
  //   case MCP9600_ADCRESOLUTION_18:   Serial.print("18"); break;
  //   case MCP9600_ADCRESOLUTION_16:   Serial.print("16"); break;
  //   case MCP9600_ADCRESOLUTION_14:   Serial.print("14"); break;
  //   case MCP9600_ADCRESOLUTION_12:   Serial.print("12"); break;
  // }
  // Serial.println(" bits");

  // mcp.setThermocoupleType(MCP9600_TYPE_K);
  // Serial.print("Thermocouple type set to ");
  // switch (mcp.getThermocoupleType()) {
  //   case MCP9600_TYPE_K:  Serial.print("K"); break;
  //   case MCP9600_TYPE_J:  Serial.print("J"); break;
  //   case MCP9600_TYPE_T:  Serial.print("T"); break;
  //   case MCP9600_TYPE_N:  Serial.print("N"); break;
  //   case MCP9600_TYPE_S:  Serial.print("S"); break;
  //   case MCP9600_TYPE_E:  Serial.print("E"); break;
  //   case MCP9600_TYPE_B:  Serial.print("B"); break;
  //   case MCP9600_TYPE_R:  Serial.print("R"); break;
  // }
  // Serial.println(" type");

  // mcp.setFilterCoefficient(3);
  // Serial.print("Filter coefficient value set to: ");
  // Serial.println(mcp.getFilterCoefficient());

  // mcp.setAlertTemperature(1, 30);
  // Serial.print("Alert #1 temperature set to ");
  // Serial.println(mcp.getAlertTemperature(1));
  // mcp.configureAlert(1, true, true);  // alert 1 enabled, rising temp

  mcp.enable(true);

  greenLED();
}

void scanWIFI()
{

  // WiFi.scanNetworks will return the number of networks found.
  int n = WiFi.scanNetworks();
  Serial.println("Scan done");
  if (n == 0)
  {
    Serial.println("no networks found");
  }
  else
  {
    Serial.print(n);
    Serial.println(" networks found");
    Serial.println("Nr | SSID                             | RSSI | CH | Encryption");
    for (int i = 0; i < n; ++i)
    {
      // Print SSID and RSSI for each network found
      Serial.printf("%2d", i + 1);
      Serial.print(" | ");
      Serial.printf("%-32.32s", WiFi.SSID(i).c_str());
      Serial.print(" | ");
      Serial.printf("%4ld", WiFi.RSSI(i));
      Serial.print(" | ");
      Serial.printf("%2ld", WiFi.channel(i));
      Serial.print(" | ");
      switch (WiFi.encryptionType(i))
      {
      case WIFI_AUTH_OPEN:
        Serial.print("open");
        break;
      case WIFI_AUTH_WEP:
        Serial.print("WEP");
        break;
      case WIFI_AUTH_WPA_PSK:
        Serial.print("WPA");
        break;
      case WIFI_AUTH_WPA2_PSK:
        Serial.print("WPA2");
        break;
      case WIFI_AUTH_WPA_WPA2_PSK:
        Serial.print("WPA+WPA2");
        break;
      case WIFI_AUTH_WPA2_ENTERPRISE:
        Serial.print("WPA2-EAP");
        break;
      case WIFI_AUTH_WPA3_PSK:
        Serial.print("WPA3");
        break;
      case WIFI_AUTH_WPA2_WPA3_PSK:
        Serial.print("WPA2+WPA3");
        break;
      case WIFI_AUTH_WAPI_PSK:
        Serial.print("WAPI");
        break;
      default:
        Serial.print("unknown");
      }
      Serial.println();
      delay(10);
    }
  }
  Serial.println("");

  // Delete the scan result to free memory for code below.
  WiFi.scanDelete();
}

int scanned = 0;
float prevTemp = 0.0;
int counter = 0;

void loop()
{
  delay(1000);

  float temp = readTemperature();

  if (temp == 0)
  {
    redLED();
    return;
  }

  greenLED();

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0, 0);
  display.println("found the data:");
  display.println(temp);
  display.display(); // actually display all of the above

  if (temp > prevTemp + 1.0 || temp < prevTemp - 1.0 && counter % 60 == 0)
  {
    Serial.println("Temperature changed by more than 1 degree");
    postTemperature(temp);
    prevTemp = temp;
  }
  counter++;
}