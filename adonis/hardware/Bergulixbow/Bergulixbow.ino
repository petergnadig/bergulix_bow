/* Adonis JS wesocket protocol doc: https://github.com/adonisjs/adonis-websocket-protocol
 *  
 */

#include <ESP8266WiFiMulti.h>
#include <WiFiUdp.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>

#include <FS.h>
#include <time.h>
#include <string.h>

#include "src/libs/mpu6050.h"

/********************************************************
                        VARIABLES
 *********************************************************/

#define SERIAL_SPEED                      115200
#define FW_VERSION                        "0.1"

#define WIFI_SSID                         "Band-Csik5"
#define WIFI_PASSWORD                     "RT-AC66UB1"

#define SERVER                            "192.168.1.12"
#define FTP_PORT                          2121
#define UDP_PORT                          37666
#define WEBSOCKET_PORT                    3333
#define WEBSOCKET_URI                     "/adonis-ws"
#define WEBSOCKET_TOPIC                   "measurement"
#define WEBSOCKET_PING_PERIOD             8000

#define ADONIS_WS_P_OPEN                  0
#define ADONIS_WS_P_JOIN                  1
#define ADONIS_WS_P_LEAVE                 2
#define ADONIS_WS_P_JOIN_ACK              3
#define ADONIS_WS_P_JOIN_ERROR            4
#define ADONIS_WS_P_LEAVE_ACK             5
#define ADONIS_WS_P_LEAVE_ERROR           6
#define ADONIS_WS_P_EVENT                 7
#define ADONIS_WS_P_PING                  8
#define ADONIS_WS_P_PONG                  9

#define UDP_PACKET_SEPARATOR              ','
#define UDP_PACKET_SENDER                 0
#define UDP_PACKET_PHASE                  1
#define UDP_PACKET_SAMPLES                2
#define UDP_PACKET_M_ID                   3

#define FILE_READ                         "r"
#define FILE_WRITE                        "w"
#define FILE_APPEND                       "a"

#define SWAP(x, y) swap = x; x = y; y = swap

// Chip properties
String chipSerial;
char chipSerialC[40];

// Timer
unsigned long time0;
unsigned long time1;

unsigned int measurement_id;

// IP
IPAddress ipaddress;
IPAddress broadcast;
IPAddress netmask;

// Connection states
boolean wifiIsConnected = false;
boolean udpIsConnected = false;

// UDP
WiFiUDP Udp;
char udpPacketBuffer[255];

// Websocket
WebSocketsClient webSocket;

// FTP buffers
char outBuf[128];
char outCount;

// Wifi
ESP8266WiFiMulti WiFiMulti;

// TCP clients
WiFiClient client;
WiFiClient dclient;

// File handler
String filename;
File fh;

// Gyro data
typedef union GyroData {
  struct
  {
    uint8_t x_accel_h;
    uint8_t x_accel_l;
    uint8_t y_accel_h;
    uint8_t y_accel_l;
    uint8_t z_accel_h;
    uint8_t z_accel_l;
  } reg;
  struct
  {
    int16_t x_accel;
    int16_t y_accel;
    int16_t z_accel;
  } value;
};

/********************************************************
                        SETUP BLOCK
 *********************************************************/

void setup()
{

  // Setup serial speed
  Serial.begin(SERIAL_SPEED);
  Serial.setDebugOutput(true);

  // setup random seed
  randomSeed(analogRead(0));

  // Waiting for serial init
  while (!Serial)
    continue;

  // Setup blue led
  pinMode(LED_BUILTIN, OUTPUT);

  // Setup IMU sensor
  imuSetup();

  // Mount flash filesystem
  SPIFFS.begin();

  // Get chip serial number
  chipSerial = String(ESP.getChipId(), HEX);
  filename = chipSerial + ".csv";

  // Print basic informations to serial
  Serial.println("Booting...");
  Serial.printf("Firmware version: %s\n", FW_VERSION);
  Serial.print("ChipSerial: ");
  Serial.println(chipSerial);

  WiFiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);
  while (WiFiMulti.run() != WL_CONNECTED) 
    delay(100);

  // Get ip address and netmask then calculate network broadcast address
  ipaddress = WiFi.localIP();
  netmask = WiFi.subnetMask();
  broadcast = calculateBroadcast(ipaddress, netmask);
  Serial.print("broadcast:");
  Serial.println(broadcast.toString());
  Serial.print("Server:" + String(SERVER));
  Serial.print(" Udp port:" + String(UDP_PORT));
  Serial.println(" Ftp port:" + String(FTP_PORT));

  // init udp
  connectUdp(UDP_PORT);

  // login to server via websocket
  webSocket.begin(SERVER, WEBSOCKET_PORT, WEBSOCKET_URI);

  // add websocket event handler
  webSocket.onEvent(webSocketEvent);

  // try every 5 sec again if connection has failed
  webSocket.setReconnectInterval(5000);

  // and end of the boot section.
  Serial.println("Boot complete.");
}

/********************************************************
                      MAIN LOOP BLOCK
 *********************************************************/
String serialCommand;
void loop()
{

  listenUdpMessage();
  webSocket.loop();

  // some test functionality on serial port
  if (Serial.available())
  {
    serialCommand = Serial.readStringUntil('\n');

    if (serialCommand.equals("m"))
      doMeasurement(1000);
    if (serialCommand.equals("upload"))
      sendDataToFTPServer();

    if (serialCommand.equals("format"))
    {
      Serial.println("Start formatting...");
      bool formatted = SPIFFS.format();

      if (formatted)
        Serial.print("success.");
      else
        Serial.println("\n\nError formatting");
    }
  }
}

/********************************************************
                        UDP BLOCK
 ********************************************************/
// Calculate broadcast ip address
IPAddress calculateBroadcast(IPAddress ipaddress, IPAddress netmask)
{
  IPAddress broadcast;
  broadcast[0] = ipaddress[0] | (~netmask[0]);
  broadcast[1] = ipaddress[1] | (~netmask[1]);
  broadcast[2] = ipaddress[2] | (~netmask[2]);
  broadcast[3] = ipaddress[3] | (~netmask[3]);
  return broadcast;
}

// Catch UDP message and parse the message parameter's
void listenUdpMessage()
{
  int packetSize = Udp.parsePacket();

  if (packetSize)
  {
    // Read the packet data into udpPacketBufffer
    int len = Udp.read(udpPacketBuffer, 255);
    if (len > 0)
      udpPacketBuffer[len] = 0;

    // then parse UDP message contents
    String sender = parseUdpMessage(udpPacketBuffer, UDP_PACKET_SEPARATOR, UDP_PACKET_SENDER);
    String phase = parseUdpMessage(udpPacketBuffer, UDP_PACKET_SEPARATOR, UDP_PACKET_PHASE);
    String samples = parseUdpMessage(udpPacketBuffer, UDP_PACKET_SEPARATOR, UDP_PACKET_SAMPLES);
    String m_id = parseUdpMessage(udpPacketBuffer, UDP_PACKET_SEPARATOR, UDP_PACKET_M_ID);

    sender.trim();
    phase.trim();
    samples.trim();
    m_id.trim();

    measurement_id = m_id.toInt();

    // then print UDP message contents to serial
    Serial.printf("[UDP] Sender: %s | ", sender.c_str());
    Serial.printf("Phase: %s | ", phase.c_str());
    Serial.printf("Samples: %s | ", samples.c_str());
    Serial.printf("Measurement id: %s\n", m_id.c_str());

    // then run command
    if (phase == "START")
    {
      wsSendBroadcastMessage("MEASUREMENT_STARTED");
      doMeasurement(samples.toInt());
      wsSendBroadcastMessage("FTP_UPLOAD_START");
      sendDataToFTPServer();
      wsSendBroadcastMessage("FTP_UPLOAD_COMPLETE");
    }
  }
}

// Create UDP connection and return state boolean
boolean connectUdp(unsigned port)
{
  boolean state = false;
  Serial.print("Connecting to UDP...");

  if (Udp.begin(port) == 1)
  {
    Serial.println("success.");
    state = true;
  }
  else
  {
    Serial.println("Connection failed.");
  }

  return state;
}

// General UDP sender
//void sendUdpPacket(IPAddress address, int port, char* message, int size) {
//  Udp.beginPacket(address, port);
//  Udp.write(message, size);
//  Udp.endPacket();
//}
//
//// Send UDP message to broadcast ip address
//void sendUdpMessage(String message) {
//  memset(udpPacketBuffer, 0, sizeof udpPacketBuffer);
//  message = chipSerial + "," + message;
//  message.toCharArray(udpPacketBuffer, 255);
//  sendUdpPacket(broadcast, UDP_PORT, udpPacketBuffer, sizeof(udpPacketBuffer));
//}

// Parse the received UDP message
String parseUdpMessage(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length();

  for (int i = 0; i <= maxIndex && found <= index; i++)
  {
    if (data.charAt(i) == separator || i == maxIndex)
    {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

/********************************************************
                        WEBSOCKET BLOCK
 ********************************************************/

void wsSendBroadcastMessage(String phase)
{
  StaticJsonDocument<200> doc;

  if (phase == "LOGIN")
    measurement_id;

  doc["t"] = ADONIS_WS_P_EVENT;
  JsonObject d = doc.createNestedObject("d");
  d["topic"] = WEBSOCKET_TOPIC;
  d["event"] = "message";

  JsonObject msgData = d.createNestedObject("data");
  msgData["sender"] = chipSerial;
  msgData["phase"] = phase;
  msgData["measurement_id"] = measurement_id;

  String result = doc.as<String>();
  webSocket.sendTXT(result.c_str());
}

void wsSendStatusMessage(int type)
{
  StaticJsonDocument<200> doc;

  doc["t"] = type;
  JsonObject d = doc.createNestedObject("d");
  d["topic"] = WEBSOCKET_TOPIC;

  String result = doc.as<String>();
  webSocket.sendTXT(result.c_str());
}

void webSocketEvent(WStype_t wsType, uint8_t *payload, size_t length)
{

  switch (wsType)
  {

  case WStype_DISCONNECTED:
    Serial.printf("[WS] Disconnected!\n");
    break;

  case WStype_CONNECTED:
    Serial.printf("[WS] Connected to url: %s\n", payload);
    break;

  case WStype_TEXT:
    // for DEBUGING purpose
    //Serial.printf("[WS] raw message: %s\n", payload);

    StaticJsonDocument<300> doc;
    deserializeJson(doc, payload);

    const int type = doc["t"];
    const String sender = doc["d"]["data"]["sender"];
    const String phase = doc["d"]["data"]["phase"];
    const unsigned int samples = doc["d"]["data"]["samples"];

    switch (type)
    {

    case ADONIS_WS_P_OPEN:
      Serial.println("[WS] OPEN sent");
      // send join message to the predefinied channel
      wsSendStatusMessage(ADONIS_WS_P_JOIN);
      break;

    case ADONIS_WS_P_JOIN_ACK:
      Serial.println("[WS] JOIN_ACK received");
      
      // send login message over websocket
      wsSendBroadcastMessage("LOGIN sent");

      break;

    case ADONIS_WS_P_EVENT:
      break;

    case ADONIS_WS_P_PONG:
      Serial.println("[WS] PONG received");
      break;
    }

    break;
  }
}

// currently not in use
//void webSocketPing() {
//  // send ping over websocket to keep alive the TCP connection
//  wsSendStatusMessage(ADONIS_WS_P_PING);
//  Serial.println("[WS] PING sent");
//}

/********************************************************
                        IMU BLOCK
 ********************************************************/

String imuSetup()
{

  int error;
  uint8_t c;

  // Wake up sensor
  Wire.begin();

  MPU6050_write_reg(MPU6050_PWR_MGMT_1, 0);

  error = MPU6050_read(MPU6050_WHO_AM_I, &c, 1);

  Serial.print(F("WHO_AM_I : "));
  Serial.print(c, HEX);
  Serial.print(F(", error = "));
  Serial.println(error, DEC);

  error = MPU6050_read(MPU6050_PWR_MGMT_1, &c, 1);

  Serial.print(F("PWR_MGMT_1 : "));
  Serial.print(c, HEX);
  Serial.print(F(", error = "));
  Serial.println(error, DEC);

  // MPU-6050 setup & read setup
  uint8_t mpu_config;
  uint8_t mpu_accel;
  uint8_t mpu_config_dlpf = mpu_config & 0b00000111;
  uint8_t mpu_config_esyn = (mpu_config & 0b00111000) >> 3;

  error = MPU6050_read(MPU6050_CONFIG, (uint8_t *)&mpu_config, (bool)1);
  Serial.println(error, BIN);

  error = MPU6050_read(MPU6050_ACCEL_CONFIG, (uint8_t *)&mpu_accel, (bool)1);
  Serial.println(error, BIN);

  Serial.print("Config      : ");
  Serial.println(mpu_config, BIN);
  Serial.print("Accel setup : ");
  Serial.println(mpu_accel, BIN);
  Serial.print("Config dlpf : ");
  Serial.println(mpu_config_dlpf, DEC);
  Serial.print("Config esyn : ");
  Serial.println(mpu_config_esyn, DEC);
  Serial.println("IMU init done.");
}

// Start the measurement process
void doMeasurement(unsigned int samples)
{

  // turn blue LED on.
  digitalWrite(LED_BUILTIN, LOW);

  int error;
  uint8_t swap;
  String message;
  GyroData data;
  unsigned long timeDiff = 0;

  // Delete old measurement data file
  SPIFFS.remove(String("/") + filename);

  // I2C init
  Wire.begin();

  // Get uptime to time0
  time0 = millis();

  Serial.println("[MMNT] Start time: " + String(time0) + " samples: " + String(samples));
  fh = SPIFFS.open(String("/") + filename, FILE_APPEND);

  if (!fh)
  {
    Serial.println("File append open failed");
  }

  // insert headers of the csv (these will be the database columns)
  fh.print("imu_serial,time_raw,acc_x,acc_y,acc_z\n");

  // Read IMU data and write the data to flash filesystem for predefinied numbers
  for (unsigned int count = 1; count <= samples; count++)
  {

    // do the measurement
    error = MPU6050_read(MPU6050_ACCEL_XOUT_H, (uint8_t *)&data, 6);

    // and get uptime to time1
    time1 = millis();

    // Swap two data
    SWAP(data.reg.x_accel_h, data.reg.x_accel_l);
    SWAP(data.reg.y_accel_h, data.reg.y_accel_l);
    SWAP(data.reg.z_accel_h, data.reg.z_accel_l);

    // Calculate time differention
    timeDiff = time1 - time0;

    // Convert to json object and save it
    fh.print(gyroDataToCsv(timeDiff, data));

    //Pass control to other tasks
    yield();
  }
  fh.print("END");

  Serial.print("[MMNT] end time: " + String(timeDiff) + "| File size: ");
  Serial.println(fh.size());
  fh.close();

  // turn LED off
  digitalWrite(LED_BUILTIN, HIGH);
}

String gyroDataToCsv(unsigned long time, GyroData data)
{
  String sep = ",";
  String result = chipSerial + sep + time + sep + data.value.x_accel + sep + data.value.y_accel + sep + data.value.z_accel + "\n";
  return result;
}

/********************************************************
                        FTP BLOCK
 *********************************************************/

byte sendDataToFTPServer()
{
  fh = SPIFFS.open(String("/") + filename, FILE_READ);

  if (!fh)
  {
    Serial.println(F("File open fail."));
    return 0;
  }

  if (client.connect(SERVER, FTP_PORT))
  {
    Serial.println("[FTP] command port connected.");
  }
  else
  {
    Serial.println("[FTP] command port connection failed.");
    fh.close();
    return 0;
  }

  if (!eFTPRcv())
    return 0;
  client.write("USER test\r\n");
  if (!eFTPRcv())
    return 0;
  client.write("PASS test\r\n");
  if (!eFTPRcv())
    return 0;
  client.write("SYST\r\n");
  if (!eFTPRcv())
    return 0;
  client.write("PASV\r\n");
  if (!eFTPRcv())
    return 0;

  char *tStr = strtok(outBuf, "(,");
  int array_pasv[6];
  for (int i = 0; i < 6; i++)
  {
    tStr = strtok(NULL, "(,");
    array_pasv[i] = atoi(tStr);
    if (tStr == NULL)
    {
      Serial.println(F("Bad PASV Answer"));
    }
  }

  unsigned int hiPort, loPort;

  hiPort = array_pasv[4] << 8;
  loPort = array_pasv[5] & 255;

  hiPort = hiPort | loPort;
  Serial.print(F("[FTP] data port: "));
  Serial.println(hiPort);

  if (dclient.connect(SERVER, hiPort))
  {
    Serial.println("[FTP] data port connected.");
  }
  else
  {

    String message = "FTP_UPLOAD_FAILED, Data port connection failed.";
    wsSendBroadcastMessage("FTP_UPLOAD_FAILED");
    Serial.println(message);

    client.stop();
    fh.close();
    return 0;
  }

  client.println(String("STOR ") + filename);

  if (!eFTPRcv())
  {
    dclient.stop();
    return 0;
  }

  byte clientBuf[64];
  int clientCount = 0;

  while (fh.available())
  {
    yield();
    clientBuf[clientCount] = fh.read();
    clientCount++;

    if (clientCount > 63)
    {
      dclient.write(clientBuf, 64);
      clientCount = 0;
    }
  }

  if (clientCount > 0)
    dclient.write(clientBuf, clientCount);

  dclient.stop();
  if (!eFTPRcv())
    return 0;
  Serial.println(F("[FTP] data port disconnected."));

  client.write("QUIT\r\n");
  if (!eFTPRcv())
    return 0;

  dclient.stop();
  client.stop();
  Serial.println("[FTP] command port disconnected.");

  fh.close();
  return 1;
}

byte eFTPRcv()
{
  byte respCode;
  byte thisByte;

  while (!client.available())
    delay(1);
  respCode = client.peek();
  outCount = 0;

  while (client.available())
  {
    thisByte = client.read();
    Serial.write(thisByte);

    if (outCount < 127)
    {
      outBuf[outCount] = thisByte;
      outCount++;
      outBuf[outCount] = 0;
    }
  }

  if (respCode >= '4')
  {
    eFTPFail();
    return 0;
  }

  return 1;
}

void eFTPFail()
{
  byte thisByte = 0;

  client.write("QUIT\r\n");
  while (!client.available())
    delay(1);
  while (client.available())
  {
    thisByte = client.read();
    Serial.write(thisByte);
  }

  client.stop();
  dclient.stop();
  String message = "FTP_UPLOAD_FAILED, Something wrong...FTP disconnected.";
  wsSendBroadcastMessage("FTP_UPLOAD_FAILED");
  Serial.println(message);
}
