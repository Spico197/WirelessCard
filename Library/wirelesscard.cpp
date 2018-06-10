#include "wirelesscard.h"


/*
 * @intro: 读取卡号并将卡号转换为String类型后返回
 * @return <String>: 卡号
 */
String Card::readUID()
{
    // 读卡部分，卡号直接存在rfid.uid.uidByte数组为字节型数组
    if (MFRC522::PICC_IsNewCardPresent())
    {
        if (MFRC522::PICC_ReadCardSerial())
        {
            MFRC522::PICC_HaltA();
            MFRC522::PCD_StopCrypto1();
            String str, temp;
            for (char i = 0; i < 4; i++)
            {
                temp = String(MFRC522::uid.uidByte[i], HEX);
                if (temp.length() == 1)
                    temp = '0' + temp;
                str += temp;
            }
            return str;
        }
    }
    return "-1";
}

/*
 * @intro:WIFI模块初始化
 * @param <String> WiFiSSID:WIFI热点名称
 * @param <String> WiFiPASSWORD: WIFI热点密码
 */
void Card::WIFIInit(String WiFiSSID, String WiFiPASSWORD)
{
    DebugSerial.println("Setting start");
    //ESP8266通电启动等待
    delay(10000);
    //如果是透传模式，退出透传
    DebugSerial.println("Exit pass-through mode");
    WIFISerial.print("+++");
    delay(1000);
    WIFISerial.print("AT\r\n");
    delay(1000);
    wifiSerialPrint();
    //关闭命令回显
    DebugSerial.println("Close command echo");
    WIFISerial.print("ATE0\r\n");
    delay(1000);
    wifiSerialPrint();
    //设置WiFi应用模式为 Station
    DebugSerial.println("Choose station mode");
    WIFISerial.print("AT+CWMODE=1\r\n");
    delay(1000);
    wifiSerialPrint();
    DebugSerial.println("Connect to wireless router");
    WIFISerial.print("AT+CWJAP=\"");
    WIFISerial.print(WiFiSSID);
    WIFISerial.print("\",\"");
    WIFISerial.print(WiFiPASSWORD);
    WIFISerial.print("\"\r\n");
    delay(10000);
    wifiSerialPrint();
    DebugSerial.println("Connect to Server");
    WIFISerial.print("AT+CIPSTART=\"TCP\",\"39.108.7.66\",2333\r\n");
    delay(8000);
    wifiSerialPrint();
    //设置模块传输模式为透传模式
    DebugSerial.println("Choose pass-through mode");
    WIFISerial.print("AT+CIPMODE=1\r\n");
    delay(3000);
    wifiSerialPrint();
    //进入透传模式
    DebugSerial.println("Enter pass-through mode");
    WIFISerial.print("AT+CIPSEND\r\n");
    delay(1000);
    wifiSerialPrint();
    DebugSerial.println("Setting over");
    delay(2000);
}

/*
 * @intro: 向屏幕发送指令信息，以改变屏幕信息。
 * @example: card.ScreenCmd("title", "你好"); // 把title模块的文本信息改为“你好”
 * @param <String> position: 需更改的模块，如"title"
 * @param <String> text: 更改为什么内容
 */
void Screen::ScreenCmd(String position, String text)
{
    String cmd = position + ".txt=" + '"' + text + '"';
    ScreenSerial.write(cmd.c_str());
    ScreenSerial.write(0xff);
    ScreenSerial.write(0xff);
    ScreenSerial.write(0xff);
}

/*
 * @intro: 读取屏幕返回的信息
 * @return <byte>: 返回屏幕操作后的返回代码
 */
byte Screen::ScreenRead()
{
    if (ScreenSerial.available())
        return byte(ScreenSerial.read());
}

/*
 * @intro: 构建非管理员模式下的json数据
 * @param <String> deviceID: 设备号
 * @param <String> uid: 卡号UID
 * @param <int> action: 动作号，即模式号
 */
void Card::jsonConstruct(String deviceID, String uid, int action)
{
    jsonData["device_id"] = deviceID;
    jsonData["uid"] = uid;
    jsonData["action"] = action;
    jsonAdmin["admin_uid"] = "";
    jsonAdmin["admin_pswd"] = "";
}

/*
 * @intro: 构建json数据
 * @param <String> deviceID: 设备号
 * @param <String> uid: 卡号UID
 * @param <int> action: 动作号，即模式号
 * @param <String> adminUID: 管理员模式下的管理员卡号
 * @param <String> adminPSWD: 管理员模式下管理员账号的密码
 */
void Card::jsonConstruct(String deviceID, String uid, int action, String adminUID, String adminPSWD)
{
    jsonData["device_id"] = deviceID;
    jsonData["uid"] = uid;
    jsonData["action"] = action;
    jsonAdmin["admin_uid"] = adminUID;
    jsonAdmin["admin_pswd"] = adminPSWD;
}

/*
 * @intro: 上传json数据
 */
void Card::jsonDataUpdate()
{
    WIFISerial.print("POST /data_post HTTP/1.1\r\n");
    WIFISerial.print("Host: 39.108.7.66\r\n");
    WIFISerial.print("Content-Type: application/json\r\n");
    WIFISerial.print("Content-Length: " + String(jsonData.measureLength()) + "\r\n");
    WIFISerial.print("\r\n");
    jsonData.printTo(WIFISerial);
    WIFISerial.print("\r\n");
}

/*
 * @intro: 构建并上传json数据
 * @param <String> deviceID: 设备号
 * @param <String> uid: 卡号UID
 * @param <int> action: 动作号，即模式号
 * @param <String> adminUID: 管理员模式下的管理员卡号
 * @param <String> adminPSWD: 管理员模式下管理员账号的密码
 */
void Card::jsonDataUpdate(String deviceID, String uid, int action, String adminUID, String adminPSWD)
{
    jsonConstruct(deviceID, uid, action, adminUID, adminPSWD);
    WIFISerial.print("POST /data_post HTTP/1.1\r\n");
    WIFISerial.print("Host: 39.108.7.66\r\n");
    WIFISerial.print("Content-Type: application/json\r\n");
    WIFISerial.print("Content-Length: " + String(jsonData.measureLength()) + "\r\n");
    WIFISerial.print("\r\n");
    jsonData.printTo(WIFISerial);
    WIFISerial.print("\r\n");
}

/*
 * @intro: 将错误码转换为错误信息
 * @param <int> ch: 错误码
 */
String Card::errorNo2Info(int ch)
{ 
    String re;
    switch (ch)
    {
        case 0: re = "上传成功"; break;
        case 1: re = "数据处理失败"; break;
        case 2: re = "用户不存在"; break;
        case 3: re = "卡号错误"; break;
        case 4: re = "管理员账号错误"; break;
        case 5: re = "模式错误"; break;
        case 6: re = "设备号错误"; break;
        case 7: re = "数据格式错误"; break;
        case 8: re = "请求错误"; break;
        case 9: re = "账户已存在"; break;
        default: re = "用户已存在"; break;
    }
    return re;
}

/*
 * @intro: 查看返回的json数据
 * @return <int>: 返回错误码，若返回-1，则返回值解析失败
 */
int Card::jsonDataReturn()
{
    char temp, pos1, pos2;
    String inputString, jsonString;
    // memset(esp8266buffer, 0, 256);
    // WIFISerial.readBytes(esp8266buffer, 256);
    for (unsigned char ii = 0; ii < 256; ii++) {
        if (WIFISerial.available())
            inputString += char(WIFISerial.read());
        else
            break;
    }
    // debug statement: output the inputString:
    // DebugSerial.println(inputString);
    if (inputString.substring("HTTP/1.1 200 OK") != -1)
    {
        pos1 = inputString.indexOf('{');
        pos2 = inputString.indexOf('}');
        if (pos1 != -1 && pos2 != -1 && pos2 > pos1)
        {
            jsonString = inputString.substring(pos1, pos2 - pos1 + 1);
            JsonObject& reJson = returnJsonBuffer.parseObject(jsonString);
            if (reJson.success())
                return int(reJson["error"]);
        }
    }
    return -1;
}

/*
 * @intro: Debug串口打印WIFI串口信息
 */
void Card::wifiSerialPrint()
{
    while (WIFISerial.available()) {
        DebugSerial.print((char)WIFISerial.read());
    }
    DebugSerial.println();
}

void Card::jsonDebugPrint()
{
    jsonData.printTo(DebugSerial);
}
