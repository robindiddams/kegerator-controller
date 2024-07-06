#include "Adafruit_MCP9601.h"
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

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





#define I2C_ADDRESS (0x67)

Adafruit_MCP9601 mcp;

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }

  Serial.println("128x64 OLED FeatherWing test");
  delay(250); // wait for the OLED to power up
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
  Serial.println("Button test");

  // pinMode(BUTTON_A, INPUT_PULLUP);
  // pinMode(BUTTON_B, INPUT_PULLUP);
  // pinMode(BUTTON_C, INPUT_PULLUP);

  // text display tests
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0,0);
  display.print("Connecting to SSID\n'adafruit':");
  display.print("connected!");
  display.println("IP: 10.0.1.23");
  display.println("Sending val #0");
  display.display(); // actually display all of the above

  Serial.println("Adafruit MCP9601 test");

  /* Initialise the driver with I2C_ADDRESS and the default I2C bus. */
  if (! mcp.begin(I2C_ADDRESS, &Wire1)) {
      Serial.println("Sensor not found. Check wiring!");
      while (1);
  }

  Serial.println("Found MCP9601!");

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

}

void loop()
{
  uint8_t status = mcp.getStatus();
  Serial.print("MCP Status: 0x"); 
  Serial.print(status, HEX);  
  Serial.print(": ");
  if (status & MCP9601_STATUS_OPENCIRCUIT) { 
    Serial.println("Thermocouple open!"); 
    return; // don't continue, since there's no thermocouple
  }
  if (status & MCP9601_STATUS_SHORTCIRCUIT) { 
    Serial.println("Thermocouple shorted to ground!"); 
    return; // don't continue, since the sensor is not working
  }
  if (status & MCP960X_STATUS_ALERT1) { Serial.print("Alert 1, "); }
  if (status & MCP960X_STATUS_ALERT2) { Serial.print("Alert 2, "); }
  if (status & MCP960X_STATUS_ALERT3) { Serial.print("Alert 3, "); }
  if (status & MCP960X_STATUS_ALERT4) { Serial.print("Alert 4, "); }
  Serial.println();
  
  float temp = mcp.readThermocouple();
  Serial.print("Hot Junction: "); Serial.println(temp);
  Serial.print("Cold Junction: "); Serial.println(mcp.readAmbient());
  Serial.print("ADC: "); Serial.print(mcp.readADC() * 2); Serial.println(" uV");





  delay(1000);

  yield();

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0,0);
  display.println("found the data:");
  display.println(temp);
  display.display(); // actually display all of the above


}