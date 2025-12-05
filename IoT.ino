#include <WiFi.h>
#include "thingProperties.h"
#define LED_BUILTIN 2
HardwareSerial stmSerial(2);  // UART2
WiFiClient client;

void connectWifi()
{
  WiFi.begin(SSID, PASS);
  Serial.print("Connecting to WiFi");

  int retry = 0;
  while (WiFi.status() != WL_CONNECTED && retry < 10) {
    delay(500);
    Serial.print("Connect Wifi => Retry #");
    Serial.println(retry);
    retry++;
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println(WiFi.localIP());
}

bool reconnectWiFi()
{
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected. Reconnecting...");
    WiFi.disconnect();

    connectWifi();

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nWiFi reconnected.");
    } else {
      Serial.println("\nFailed to reconnect WiFi.");
      delay(5000);
      return false;
    }
  }
  return true;
}

bool connectPCServer()
{
  if (!client.connected()) {
    Serial.println("Connecting to server...");
    Serial.println(HOST);
    Serial.println(PORT);
    if (!client.connect(HOST, PORT)) {
      Serial.println("Connection failed.");
      delay(5000);
      return false;
    }
    Serial.println("Connected to server.");
  }
  return true;
}

void SendMsgFromSTM32ToPC()
{
  if (stmSerial.available()) {
    String msg = stmSerial.readStringUntil('\n');  
    msg.trim(); 
    Serial.println("=====================================");
    client.print(msg); 
    Serial.println("Receive from STM32: " + msg);
    Serial.println("=> Send to PC Server");
    Serial.println("=====================================");
  }
}

void SenderMsgFromPCToSTM32()
{
  String msg = "";
  if (client.available()) {
    msg = client.readStringUntil('\n'); 
    SendMsgFromESP32ToSTM32(msg);
  }
}

void SendMsgFromESP32ToSTM32(String msg)
{
  msg.trim(); 
  Serial.println("Reply from IOT Server: " + msg);
  Serial.println("=> Send to STM32");
  stmSerial.print(msg);
}


void setup() {
  Serial.begin(115200);
  delay(1500);

  pinMode(LED_BUILTIN, OUTPUT);
  stmSerial.begin(115200, SERIAL_8N1, 16, 17); // RX = GPIO16, TX = GPIO17（可改）
  delay(1000);

  initProperties();
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);

  setDebugMessageLevel(-1);
  ArduinoCloud.printDebugInfo();

  connectWifi();
}

void loop() {
  ArduinoCloud.update();
  if(!reconnectWiFi()) return;
  if(!connectPCServer()) return;
  SendMsgFromSTM32ToPC();
  SenderMsgFromPCToSTM32();
}


void onMsgFromPhoneChange()  
{
  if (msgFromPhone == "on") {
    digitalWrite(LED_BUILTIN, HIGH);
  } else if (msgFromPhone == "off") {
    digitalWrite(LED_BUILTIN, LOW);
  }
  else {
    SendMsgFromESP32ToSTM32(msgFromPhone);
  }
}

void HSVtoRGB(float h, float s, float v, int &r, int &g, int &b) 
{
  float c = v * s; // Chroma
  float x = c * (1 - fabs(fmod(h / 60.0, 2) - 1));
  float m = v - c;

  float r1, g1, b1;
  if (h < 60) {
    r1 = c; g1 = x; b1 = 0;
  } else if (h < 120) {
    r1 = x; g1 = c; b1 = 0;
  } else if (h < 180) {
    r1 = 0; g1 = c; b1 = x;
  } else if (h < 240) {
    r1 = 0; g1 = x; b1 = c;
  } else if (h < 300) {
    r1 = x; g1 = 0; b1 = c;
  } else {
    r1 = c; g1 = 0; b1 = x;
  }

  r = round((r1 + m) * 255);
  g = round((g1 + m) * 255);
  b = round((b1 + m) * 255);
}

void onRGBChange()  
{
  int r, g, b;
  HSVtoRGB(rGB.getHue(), rGB.getSaturation() / 100.0, rGB.getBrightness() / 100.0, r, g, b);
  String rgbMsg = String("led ") + "{\"r\":" + r + ",\"g\":" + g + ",\"b\":" + b + "}\n";
  SendMsgFromESP32ToSTM32(rgbMsg);
}

void onLightSwitchChange()
{
  String switchMsg = String("relay ") + (lightSwitch ? "1" : "0");
  SendMsgFromESP32ToSTM32(switchMsg);
}
