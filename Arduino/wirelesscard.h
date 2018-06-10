/*
   说明：本程序为2018年贵州省电子设计竞赛无线联网刷卡系统的库文件
   接线说明：
    RC522模块与Mega：
      SDA   -----  53
      SCK   -----  50
      MOSI  -----  51
      MISO  -----  48
      IRQ   -----  悬空
      GND   -----  GND
      RST   -----  5
      3.3V  -----  3.3V
    ESP8266模块与Mega：
      GND   -----  GND
      GPIO2 -----  悬空
      GPIO0 -----  悬空
      RXD   -----  TX3/14
      TXD   -----  RX3/15
      CH_PD -----  上拉电阻3.3V
      RST   -----  悬空
      VCC   -----  3.3V
    4*4矩阵键盘：
      R1 ----- 49
      R2 ----- 47
      R3 ----- 45
      R4 ----- 43
      C1 ----- 41
      C2 ----- 39
      C3 ----- 37
      C4 ----- 35
   版型：Arduino Mega 2560
   作者：Spico
   时间：2018/06/09
*/


#ifndef WIRELESS_H
#define WIRELESS_H

#include <Arduino.h>
#include <MFRC522.h>
#include <ArduinoJson.h>
#include <SPI.h>

#define DebugSerial Serial
#define ScreenSerial Serial2
#define WIFISerial Serial3


class Card: public MFRC522
{
public:
    char esp8266buffer[256];
    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& jsonData = jsonBuffer.createObject();
    JsonObject& jsonAdmin = jsonData.createNestedObject("admin");
    StaticJsonBuffer<50> returnJsonBuffer;
    
    Card(): MFRC522(SS, UINT8_MAX){};
    Card(byte resetPowerDownPin): MFRC522(SS, resetPowerDownPin){};
    Card(byte chipSelectPin, byte resetPowerDownPin): MFRC522(chipSelectPin, resetPowerDownPin){};

    String readUID();
    void WIFIInit(String WiFiSSID, String WiFiPASSWORD);
    void jsonConstruct(String deviceID, String uid, int action);
    void jsonConstruct(String deviceID, String uid, int action, String adminUID, String adminPSWD);
    void jsonDataUpdate();
    void jsonDataUpdate(String deviceID, String uid, int action, String adminUID, String adminPSWD);
    String errorNo2Info(int ch);
    int jsonDataReturn();
    void wifiSerialPrint();
    void jsonDebugPrint();
};

class Screen
{
public:
    void ScreenCmd(String position, String text);
    byte ScreenRead();
};

#endif