#include "wirelesscard.h"
#include <ArduinoJson.h>
#include <SPI.h>
#include <Keypad.h>

#define KEY_ROWS 4
#define KEY_COLS 4

#define SS_PIN 53   // RC522 SDA
#define RST_PIN 5   // RC522 RST
#define SERIAL_RX_BUFFER_SIZE 256   // 串口缓冲区大小256 bytes

String WiFiSSID = "EDC2018";
String WiFiPASSWORD = "ASDFGHJKL";
String deviceID = "321HH321";

char keys[KEY_ROWS][KEY_COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};//A B C D代表四种模式，O代表确定，H代表返回主界面。
byte rowPins[KEY_ROWS] = {49, 47, 45, 43}; //connect to the row pinouts of the kpdb
byte colPins[KEY_COLS] = {41, 39, 37, 35}; //connect to the column pinouts of the kpd
Keypad kpd = Keypad( makeKeymap(keys), rowPins, colPins, KEY_ROWS, KEY_COLS );

int mode = -1;

Card card;
Screen screen;
void setup()
{
  DebugSerial.begin(9600);
  delay(10);
  WIFISerial.begin(115200);
  delay(10);
  ScreenSerial.begin(9600);
  delay(10);
  SPI.begin();
  delay(10);
  card.PCD_Init();
  while (ScreenSerial.available()) ScreenSerial.read();
  delay(10);
  card.WIFIInit(WiFiSSID, WiFiPASSWORD, screen);
  //  String ipStatus = "";
  //  String wait = "";
  //  while (ipStatus.length() <= 30)
  //  {
  //    if (WIFISerial.available())
  //    {
  //      while (WIFISerial.available())
  //      {
  //        ipStatus += WIFISerial.read();
  //        delay(30);
  //      }
  //    }
  //    else
  //    {
  //      DebugSerial.print(".");
  //      screen.ScreenCmd("dis", wait += ".");
  //    }
  //    delay(1000);
  //  }
  //  ipStatus = ipStatus.substring(ipStatus.indexOf('{'), ipStatus.length());
  //  screen.ScreenCmd("dis", ipStatus);
  //  DebugSerial.println(ipStatus);
  while (ScreenSerial.available()) ScreenSerial.read();
  delay(1000);
}

void loop()
{
  // 读取卡号，上传并修改屏幕
  //  DebugSerial.println("InitMode: " + String(mode));
  mode = screen.ScreenRead();
  //  DebugSerial.println("ScreenRead: " + String(mode));
  //  DebugSerial.println(mode, HEX);
  if (mode == 0x00)
  {
    DebugSerial.println("Mode 0: user_data");
    //delay(1000);
    String uid = "-1";
    while (uid == "-1")
      uid = card.readUID();
    DebugSerial.println(uid);
    screen.ScreenCmd("box", uid);
    delay(500);
    card.jsonConstruct(deviceID, uid, 0); // 新建普通用户模式
    delay(1000);
    //    card.jsonData.printTo(DebugSerial);
    //    DebugSerial.println(String(card.jsonData.measureLength()));
    card.jsonDataUpdate();
    // 解析返回的json数据并在屏幕上输出
    delay(500);
    int errorNumber = card.jsonDataReturn();
    if (card.jsonData["action"] != 0)
    {
      errorNumber = 11;
    }

    DebugSerial.println(card.errorNo2Info(errorNumber));
    screen.ScreenCmd("re", card.errorNo2Info(errorNumber));
    mode = -1;
  }

  else if (mode == 0x01)
    // 读取卡号，上传并修改屏幕
  {
    DebugSerial.println("Mode 1: New Admin");
    String adminPSWD;
    char readKey;
    String keyString;
    String st3 = "Pssword";
    String st2 = "Return";
    //delay(1000);
    String uid = "-1";
    while (uid == "-1")
      uid = card.readUID();
    DebugSerial.println(uid);
    screen.ScreenCmd("box", uid);
    delay(500);

    screen.ScreenCmd("title", "Input Your Pssword");
    screen.ScreenCmd("t3", st3);
    while (keyString.length() < 6)
    {
      readKey = kpd.getKey();
      if (readKey && readKey != 'D')
      {
        //DebugSerial.println("key: " + readKey);
        keyString += readKey;
        screen.ScreenCmd("re", keyString);
      }
      else if (readKey == 'D')
      {
        keyString = keyString.substring(0, keyString.length() - 1);
        screen.ScreenCmd("re", keyString);
      }
    }
    screen.ScreenCmd("re", keyString);
    adminPSWD = keyString;
    screen.ScreenCmd("t3", st2);
    delay(500);
    card.jsonConstruct(deviceID, uid, 1, uid, adminPSWD); // 新建管理员模式
    card.jsonDataUpdate();
    // 解析返回的json数据并在屏幕上输出
    delay(500);
    int errorNumber = card.jsonDataReturn();
    if (card.jsonData["action"] != 1)
    {
      errorNumber = 11;
    }
    //    if (errorNumber != -1)
    //    {
    screen.ScreenCmd("re", card.errorNo2Info(errorNumber));
    //    }
    mode = -1;
  }

  else if (mode == 0x02)
    // 读取卡号，上传并修改屏幕
  {
    DebugSerial.println("Mode 2: New User");
    String adminPSWD;
    String keyString;
    char readKey;
    String st3 = "AMUID";
    String st4 = "PSSWORD";
    String st5 = "Return";
    String uid1 = "-1";
    String uid2 = "-1";
    while (uid1 == "-1")
      uid1 = card.readUID();
    DebugSerial.println(uid1);
    screen.ScreenCmd("box", uid1);
    delay(500);

    screen.ScreenCmd("title", "Place Admin Card");
    while (uid2 == "-1")
      uid2 = card.readUID();
    DebugSerial.println(uid2);
    screen.ScreenCmd("sub", st3);
    screen.ScreenCmd("box", uid2);
    delay(500);

    screen.ScreenCmd("title", "Input Your Pssword");
    screen.ScreenCmd("t3", st4);

    while (keyString.length() < 6)
    {
      readKey = kpd.getKey();
      if (readKey && readKey != 'D')
      {
        //DebugSerial.println("key: " + readKey);
        keyString += readKey;
        screen.ScreenCmd("re", keyString);
      }
      else if (readKey == 'D')
      {
        keyString = keyString.substring(0, keyString.length() - 1);
        screen.ScreenCmd("re", keyString);
      }
    }
    screen.ScreenCmd("re", keyString);
    adminPSWD = keyString;
    screen.ScreenCmd("t3", st5);

    delay(500);
    card.jsonConstruct(deviceID, uid1, 2, uid2, adminPSWD); // 管理员新建普通用户模式
    card.jsonDataUpdate();
    // 解析返回的json数据并在屏幕上输出
    delay(500);
    int errorNumber = card.jsonDataReturn();
    if (card.jsonData["action"] != 2)
    {
      errorNumber = 11;
    }
    //    if (errorNumber != -1)
    //    {
    screen.ScreenCmd("re", card.errorNo2Info(errorNumber));
    //    }
    delay(5000);
  }

  else if (mode == 0x03)
  {
    DebugSerial.println("Mode 3: Delete User Remain Data");
    String adminPSWD;
    String keyString;
    char readKey;
    String st3 = "AMUID";
    String st4 = "PSSWORD";
    String st5 = "Return";
    String uid1 = "-1";
    String uid2 = "-1";
    while (uid1 == "-1")
      uid1 = card.readUID();
    DebugSerial.println(uid1);
    screen.ScreenCmd("box", uid1);
    delay(500);
    screen.ScreenCmd("title", "Place Admin Card");
    while (uid2 == "-1")
      uid2 = card.readUID();
    DebugSerial.println(uid2);
    screen.ScreenCmd("sub", st3);
    screen.ScreenCmd("box", uid2);
    delay(500);
    screen.ScreenCmd("title", "Input Your Pssword");
    screen.ScreenCmd("t3", st4);
    while (keyString.length() < 6)
    {
      readKey = kpd.getKey();
      if (readKey && readKey != 'D')
      {
        //DebugSerial.println("key: " + readKey);
        keyString += readKey;
        screen.ScreenCmd("re", keyString);
      }
      else if (readKey == 'D')
      {
        keyString = keyString.substring(0, keyString.length() - 1);
        screen.ScreenCmd("re", keyString);
      }
    }
    screen.ScreenCmd("re", keyString);
    adminPSWD = keyString;
    screen.ScreenCmd("t3", st5);

    delay(500);
    card.jsonConstruct(deviceID, uid1, 3, uid2, adminPSWD); // 管理员删除用户模式
    card.jsonDataUpdate();
    delay(500);
    // 解析返回的json数据并在屏幕上输出
    int errorNumber = card.jsonDataReturn();
    if (card.jsonData["action"] != 3)
    {
      errorNumber = 11;
    }
    //    if (errorNumber != -1)
    //    {
    screen.ScreenCmd("re", card.errorNo2Info(errorNumber));
    //    }
    delay(5000);
  }
  else if (mode == 0x04)
  {
    DebugSerial.println("Mode 0: Delete User And Data");
    String adminPSWD;
    String keyString;
    char readKey;
    String st3 = "AMUID";
    String st4 = "PSSWORD";
    String st5 = "Return";
    String uid1 = "-1";
    String uid2 = "-1";
    while (uid1 == "-1")
      uid1 = card.readUID();
    DebugSerial.println(uid1);
    screen.ScreenCmd("box", uid1);
    delay(500);

    screen.ScreenCmd("title", "Place Admin Card");
    while (uid2 == "-1")
      uid2 = card.readUID();
    DebugSerial.println(uid2);
    screen.ScreenCmd("sub", st3);
    screen.ScreenCmd("box", uid2);
    delay(500);
    screen.ScreenCmd("title", "Input Your Pssword");
    screen.ScreenCmd("t3", st4);
    while (keyString.length() < 6)
    {
      readKey = kpd.getKey();
      if (readKey && readKey != 'D')
      {
        //DebugSerial.println("key: " + readKey);
        keyString += readKey;
        screen.ScreenCmd("re", keyString);
      }
      else if (readKey == 'D')
      {
        keyString = keyString.substring(0, keyString.length() - 1);
        screen.ScreenCmd("re", keyString);
      }
    }
    screen.ScreenCmd("re", keyString);
    adminPSWD = keyString;
    screen.ScreenCmd("t3", st5);

    delay(500);
    card.jsonConstruct(deviceID, uid1, 4, uid2, adminPSWD); // 管理员删除用户和数据模式
    card.jsonDataUpdate();
    // 解析返回的json数据并在屏幕上输出
    int errorNumber = card.jsonDataReturn();
    delay(500);
    if (card.jsonData["action"] != 4)
    {
      errorNumber = 11;
    }
    //    if (errorNumber != -1)
    //    {
    screen.ScreenCmd("re", card.errorNo2Info(errorNumber));
    //    }
    delay(5000);
  }
  mode = -1;
  delay(10);
}



