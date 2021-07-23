/**
  Program to read out and display
  data from xiaoxiang Smart BMS
  over Bluetooth Low Energy
  https://www.lithiumbatterypcb.com/
  Tested with original BLE module provided.
  Might work with generic BLE module when UUIDs are modified

  For TTGO lilygo T5 2.13 

  (c) Deathcult456 2021

  BASED ON :
  (c) Miroslav Kolinsky 2019
  https://www.kolins.cz


*/

//#define TRACE commSerial.println(__FUNCTION__)
#define TRACE

#define LILYGO_T5_V213
#include <boards.h>
#include <GxEPD.h>
#include <FS.h>
#include <GxDEPG0213BN/GxDEPG0213BN.h>
#include <esp_wifi.h>

//#define SIMULATION
#include <Arduino.h>
#include "BLEDevice.h"
#include "mydatatypes.h"
#include <SPI.h>

#include <Wire.h>
#include "driver/rtc_io.h"

#include <Fonts/FreeSansBold8pt7b.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeSansBold14pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <Fonts/FreeMonoBold24pt7b.h>
#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>

HardwareSerial commSerial(0);
HardwareSerial bmsSerial(1);

GxIO_Class io(SPI,  EPD_CS, EPD_DC,  EPD_RSET);
GxEPD_Class display(io, EPD_RSET, EPD_BUSY);

void print_wakeup_reason();

bool goToSleep = false;
void IRAM_ATTR ISR() {
  // goToSleep = true;
}

//---- global variables ----

static boolean doConnect = false;
static boolean BLE_client_connected = false;
static boolean doScan = false;

packBasicInfoStruct packBasicInfo; //here shall be the latest data got from BMS
packEepromStruct packEeprom;     //here shall be the latest data got from BMS
packCellInfoStruct packCellInfo;   //here shall be the latest data got from BMS

const byte cBasicInfo3 = 3; //type of packet 3= basic info
const byte cCellInfo4 = 4;  //type of packet 4= individual cell info

unsigned long previousMillis = 0;
const long interval = 4000;

bool toggle = false;
bool newPacketReceived = false;
int Moss = 0;
int newMoss = 0;

void setup()
{
  esp_wifi_stop();
  
  commSerial.begin(115200, SERIAL_8N1, 3, 1);
  bmsSerial.begin(9600, SERIAL_8N1, 21, 22);

  pinMode(22, OUTPUT);  // pull down switch batterie
  pinMode(14, OUTPUT);  // pull up switch batterie
  pinMode(19, INPUT); // switch batterie

  digitalWrite(22, HIGH); // pull down switch batterie
  digitalWrite(14, LOW); // pull up switch batterie

  attachInterrupt(digitalPinToInterrupt(0), ISR, FALLING);

  SPI.begin(EPD_SCLK, EPD_MISO, EPD_MOSI);

  delay(100);
  print_wakeup_reason();



  commSerial.println("Starting ebike dashboard application...");

  bleStartup();

  display.init();
  display.eraseDisplay();
  display.fillScreen(GxEPD_WHITE);
  display.setTextColor(GxEPD_BLACK);
  //display.update();
  display.setFont(&FreeMonoBold9pt7b);
  display.setCursor(0, 10);
  display.print("Starting...");
  display.updateWindow(0, 0, GxEPD_WIDTH, GxEPD_HEIGHT, true);
  delay(100);
}

//---------------------main loop------------------
void loop()
{
  if (goToSleep == true) {
    delay(500);
    detachInterrupt(digitalPinToInterrupt(0)); //because later used for wake up
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_0, 0);
    esp_deep_sleep_start();
  }

  bleRequestData();

  if (newPacketReceived == true)
  {
    showInfoLcd();

    Moss = packBasicInfo.MosfetStatus;
    printBasicInfo();
    printCellInfo();
    newMoss = 0;
  }//new packet

  if ( digitalRead(19) ) {
    if (Moss == 1 &  newMoss == 0) {
      mosfet_ON ();
      newMoss = 1;
      commSerial.println("Request mosfet ON");
    }
  }
  else {
    if (Moss == 3 &  newMoss ==  0) {
      mosfet_OFF ();
      newMoss = 1;
      commSerial.println("Request mosfet OFF");
    }
  }

}
//---------------------/ main loop--------------- ---
void write_request_mosfetOFF() {
  uint8_t data[9] = {0xdd, 0x5a, 0xe1, 0x02, 0x00, 0x02, 0xff, 0x1b, 0x77};
  sendCommand(data, sizeof(data));
}
void write_request_mosfetON() {
  uint8_t data[9] = {0xdd, 0x5a, 0xe1, 0x02, 0x00, 0x00, 0xff, 0x1d, 0x77};
  sendCommand(data, sizeof(data));
}
void write_request_start() {
  uint8_t data[9] = {0xdd, 0x5a, 0x00, 0x02, 0x56, 0x78, 0xff, 0x30, 0x77};
  sendCommand(data, sizeof(data));
}
void write_request_end() {
  uint8_t data[9] = {0xdd, 0x5a, 0x01, 0x02, 0x00, 0x00, 0xff, 0xfd, 0x77};
  sendCommand(data, sizeof(data));
}


void mosfet_ON () {
  write_request_start();
  delay(200);
  write_request_mosfetON();
  delay(200);
  write_request_end();
}
void mosfet_OFF () {
  write_request_start();
  delay(200);
  write_request_mosfetOFF();
  delay(200);
  write_request_end();
}

void print_wakeup_reason() {
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();
  switch (wakeup_reason)
  {
    case 1  : commSerial.println("Wakeup caused by external signal using RTC_IO"); break;
    case 2  : commSerial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case 3  : commSerial.println("Wakeup caused by touchpad"); break;
    case 4  : commSerial.println("Wakeup caused by timer"); break;
    case 5  : commSerial.println("Wakeup caused by ULP program"); break;
    default : commSerial.println("Wakeup was not caused by deep sleep"); break;
  }
}
