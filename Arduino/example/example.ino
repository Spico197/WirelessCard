#include <wirelesscard.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <Keypad.h>

#define KEY_ROWS 4
#define KEY_COLS 4

#define SS_PIN 53   // RC522 SDA
#define RST_PIN 5   // RC522 RST
#define SERIAL_RX_BUFFER_SIZE 256   // 串口缓冲区大小256 bytes

String WiFiSSID = "Sugar";
String WiFiPASSWORD = "spico123";
String deviceID = "123XX123";

char keys[KEY_ROWS][KEY_COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'}, 
    {'*', '0', '#', 'D'}
};//A B C D代表四种模式，O代表确定，H代表返回主界面。
byte rowPins[KEY_ROWS] = {49, 47, 45, 43}; //connect to the row pinouts of the kpdb
byte colPins[KEY_COLS] = {41, 39, 37, 35}; //connect to the column pinouts of the kpd
Keypad kpd = Keypad( makeKeymap(keys), rowPins, colPins, KEY_ROWS, KEY_COLS );

char readKey;
char mode;
char cmd[50];

Card card;
Screen screen;

void setup()
{
    DebugSerial.begin(9600);
    delay(10);
    WIFISerial.begin(115200);
    delay(10);
    SPI.begin();
    delay(10);
    card.PCD_Init();
    delay(10);
    card.WIFIInit(WiFiSSID, WiFiPASSWORD);
}

void loop()
{
    // 读取按键值(char类型)
    readKey = kpd.getKey();
    if(readKey)
        DebugSerial.println("key: " + readKey);

    // 读取卡号，上传并修改屏幕
    DebugSerial.println("Ready to read");
    String uid = card.readUID();
    DebugSerial.println(uid);
    screen.ScreenCmd("box", uid);
    delay(500);
    card.jsonConstruct(deviceID, uid, 2); // 新建用户模式
    card.jsonData.printTo(DebugSerial); // 输出json
    card.jsonDataUpdate();  // 上传json数据

    // 解析返回的json数据并在屏幕上输出
    int errorNumber = card.jsonDataReturn();
    DebugSerial.println("error: " + String(errorNumber));
    if (errorNumber != -1)
        screen.ScreenCmd("re", card.errorNo2Info(errorNumber));
    else
        screen.ScreenCmd("re", "Bad HTTP Request");
}
