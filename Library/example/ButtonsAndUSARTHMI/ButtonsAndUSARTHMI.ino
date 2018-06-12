#include <wirelesscard.h>
#include <Keypad.h>

#define KEY_ROWS 4
#define KEY_COLS 4
#define SERIAL_RX_BUFFER_SIZE 256   // 串口缓冲区大小256 bytes

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
String keyString = "";

Screen screen;

void setup()
{
  DebugSerial.begin(9600);
  ScreenSerial.begin(9600);
  delay(10);
  screen.ScreenCmd("dis", ">");
}

void loop()
{
  // 读取按键值(char类型)
  keyString = "";
  while (keyString.length() < 6)
  {
    readKey = kpd.getKey();
    if (readKey)
    {
      DebugSerial.println("key: " + readKey);
      keyString += readKey;
      screen.ScreenCmd("box", keyString);
    }
  }
  screen.ScreenCmd("box", keyString);
  delay(20);
}
