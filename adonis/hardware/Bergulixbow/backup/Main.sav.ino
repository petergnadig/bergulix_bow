#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <FS.h>
#include <time.h>
#include <string.h>
#include "src/libs/mpu6050.h"

/********************************************************
                        VARIABLES
 *********************************************************/

#define SERIAL_SPEED              115200
#define FW_VERSION                "0.1"

#define WIFI_SSID                 "WIFI"
#define WIFI_PASSWORD             "4iG2019Q1WiFi"

#define FTP_SERVER                "192.168.66.102"
#define FTP_PORT                  2121

#define UDP_PORT                  37666
#define UDP_PACKET_SEPARATOR      ','
#define UDP_PACKET_COMMAND        0
#define UDP_PACKET_SAMPLES        1
#define UDP_PACKET_DATETIME       2
#define UDP_PACKET_BOW            3
#define UDP_PACKET_PERSON         4

#define CMD_START_MEASUREMENT     "START"
#define CMD_SEND_LAST_DATA        "SEND_LAST"

#define MEASUREMENT_FILENAME      "measurements.csv"

#define FILE_READ                 "r"
#define FILE_WRITE                "w"
#define FILE_APPEND               "a"

#define SWAP(x,y) swap = x; x = y; y = swap

// Chip properties
String  chipSerial;
char    chipSerialC[40];

// Timer
unsigned long time0;
unsigned long time1;

// IP
IPAddress ipaddress;
IPAddress broadcast;
IPAddress netmask;

// Connection states
boolean wifiIsConnected = false;
boolean udpIsConnected  = false;

// FTP buffers
char outBuf[128];
char outCount;

// TCP clients
WiFiClient client;
WiFiClient dclient;

// Udp clients
WiFiUDP Udp;

char udpPacketBuffer[255];

// File handler
File fh;

// Gyro data
typedef union GyroData
{
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

void setup() {

  // Setup serial speed
  Serial.begin(SERIAL_SPEED);

  // setup random seed
  randomSeed(analogRead(0));

  // Waiting for serial init
  while (!Serial) continue;

  // Setup blue led
  pinMode(LED_BUILTIN, OUTPUT);

  // Get chip serial number
  chipSerial = String(ESP.getChipId(), HEX);

  // Print basic informations to serial
  Serial.println("Booting...");
  Serial.printf("Firmware version: %s\n", FW_VERSION);
  Serial.print("ChipSerial: ");
  Serial.println(chipSerial);

  // Setup IMU sensor
  imuSetup();

  // Setup WiFi mode
  WiFi.mode(WIFI_STA);

  // Trying to connect predefinied wifi
  wifiIsConnected = connectWifi(WIFI_SSID, WIFI_PASSWORD);

  // If wifi is connected
  if (wifiIsConnected) {

    // send an UDP login packet to broadcast ip
    udpIsConnected = connectUdp(UDP_PORT);
    if (udpIsConnected) sendUdpMessage(chipSerial + " ---> Login and waiting for start signal.");
  }

  // and end of the boot section.
  Serial.println("Boot complete.");

  doMeasurement(4000);
  Serial.println(F("Ready. Press u to upload file."));
  
}
  

/********************************************************
                      MAIN LOOP BLOCK
 *********************************************************/
void loop() {

  // Check wifi and UDP connection
  if (wifiIsConnected && udpIsConnected) {

    // and waiting for the UDP message
    //listenUdpMessage();
  }

  byte inChar = Serial.read();

  if (inChar == 'u') {

    if(sendFTP()) Serial.println("FTP OK");
    else Serial.println("FTP failed");
    
  }  
  delay(10);
}


/********************************************************
                        WIFI BLOCK
 *********************************************************/

boolean connectWifi(char* ssid, char* password) {
  int i = 0;
  boolean connection_state = true;

  Serial.println();
  Serial.printf("Trying connect to %s", WIFI_SSID);

  // Start wifi connection
  WiFi.begin(ssid, password);

  // and waiting for success
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (i > 40) {
      connection_state = false;
      break;
    }
    i++;
  }

  if (connection_state) {

    Serial.println(" success.");

    // Get ip address and netmask
    ipaddress = WiFi.localIP();
    netmask = WiFi.subnetMask();

    // Calculate network broadcast address
    broadcast = calculateBroadcast(ipaddress, netmask);

    // Print all information to serial
    Serial.print("IP: ");
    Serial.print(ipaddress.toString());
    Serial.print(" Netmask: ");
    Serial.print(netmask.toString());
    Serial.print(" Broadcast: ");
    Serial.println(broadcast.toString());

  }
  else {
    Serial.println("");
    Serial.println("Connection failed.");
  }

  return connection_state;
}

/********************************************************
                        UDP BLOCK
 ********************************************************/

// Catch UDP message and parse the message parameter's
void listenUdpMessage() {
  int packetSize = Udp.parsePacket();

  if (packetSize)
  {
    // Read the packet data into udpPacketBufffer
    int len = Udp.read(udpPacketBuffer, 255);
    if (len > 0) udpPacketBuffer[len] = 0;

    // then parse UDP message contents
    String command = parseUdpMessage(udpPacketBuffer, UDP_PACKET_SEPARATOR, UDP_PACKET_COMMAND );
    String samples = parseUdpMessage(udpPacketBuffer, UDP_PACKET_SEPARATOR, UDP_PACKET_SAMPLES );
    String datetime = parseUdpMessage(udpPacketBuffer, UDP_PACKET_SEPARATOR, UDP_PACKET_DATETIME );
    String bow = parseUdpMessage(udpPacketBuffer, UDP_PACKET_SEPARATOR, UDP_PACKET_BOW );
    String person = parseUdpMessage(udpPacketBuffer, UDP_PACKET_SEPARATOR, UDP_PACKET_PERSON );

    // then print UDP message contents to serial
    Serial.printf("Command: '%s' | ", command.c_str());
    Serial.printf("Samples: '%s' | ", samples.c_str());
    Serial.printf("Date: '%s' | ", datetime.c_str());
    Serial.printf("Bow: '%s' | ", bow.c_str());
    Serial.printf("Person: '%s'\n", person.c_str());

    // then run command
    if (command == CMD_START_MEASUREMENT) doMeasurement(samples.toInt());

  }
}

// Create UDP connection and return state boolean
boolean connectUdp(unsigned port) {
  boolean state = false;

  Serial.print("Connecting to UDP...");

  if (Udp.begin(port) == 1) {
    Serial.println("success.");
    state = true;
  }
  else {
    Serial.println("Connection failed.");
  }

  return state;
}

// General UDP sender
void sendUdpPacket(IPAddress address, int port, char* message, int size) {
  Udp.beginPacket(address, port);
  Udp.write(message, size);
  Udp.endPacket();
}

// Send UDP message to broadcast ip address
void sendUdpMessage(String message) {
  memset(udpPacketBuffer, 0, sizeof udpPacketBuffer);
  message.toCharArray(udpPacketBuffer, 255);
  sendUdpPacket(broadcast, UDP_PORT, udpPacketBuffer, sizeof(udpPacketBuffer));
}

// Parse the received UDP message
String parseUdpMessage(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = { 0, -1 };
  int maxIndex = data.length();

  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

// Setup IMU sensor
String imuSetup() {

  int error;
  uint8_t c;

  // Wake up sensor
  Wire.begin();

  MPU6050_write_reg (MPU6050_PWR_MGMT_1, 0);

  error = MPU6050_read (MPU6050_WHO_AM_I, &c, 1);

  Serial.print(F("WHO_AM_I : "));
  Serial.print(c, HEX);
  Serial.print(F(", error = "));
  Serial.println(error, DEC);

  error = MPU6050_read (MPU6050_PWR_MGMT_1, &c, 1);

  Serial.print(F("PWR_MGMT_1 : "));
  Serial.print(c, HEX);
  Serial.print(F(", error = "));
  Serial.println(error, DEC);

  // MPU-6050 setup & read setup
  uint8_t mpu_config;
  uint8_t mpu_accel;
  uint8_t mpu_config_dlpf = mpu_config & 0b00000111;
  uint8_t mpu_config_esyn = (mpu_config & 0b00111000) >> 3;

  error = MPU6050_read (MPU6050_CONFIG, (uint8_t *) &mpu_config, (bool) 1);
  Serial.println(error, BIN);

  error = MPU6050_read (MPU6050_ACCEL_CONFIG, (uint8_t *) &mpu_accel,  (bool) 1);
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
void doMeasurement(unsigned int samples) {

  // turn blue LED on.
  digitalWrite(LED_BUILTIN, LOW);

  int error;
  uint8_t swap;
  String message;
  GyroData data;
  unsigned long timeDiff = 0;

  // Mount flash filesystem
  SPIFFS.begin();

  // Delete old measurement data file
  SPIFFS.remove(String("/") + MEASUREMENT_FILENAME);

  // I2C init
  Wire.begin();

  // Get uptime to time0
  time0 = millis();


  printMessage("---> Measurement start time: " + String(time0) + " samples: " + String(samples));
  fh = SPIFFS.open(String("/") + MEASUREMENT_FILENAME, FILE_APPEND);

  if (!fh) {
    Serial.println("File append open failed");
  }
  
  // Read IMU data and write the data to flash filesystem for predefinied numbers
  for (unsigned int count = 1; count <= samples; count++ ) {

    // do the measurement
    error = MPU6050_read (MPU6050_ACCEL_XOUT_H, (uint8_t *) &data, 6);

    // and get uptime to time1
    time1 = millis();

    // Swap two data
    SWAP (data.reg.x_accel_h, data.reg.x_accel_l);
    SWAP (data.reg.y_accel_h, data.reg.y_accel_l);
    SWAP (data.reg.z_accel_h, data.reg.z_accel_l);

    // Calculate time differention
    timeDiff = time1 - time0; 
    
    // Convert to json object and save it
    fh.print(gyroDataToCsv(count, timeDiff, data));

    //Pass control to other tasks
    yield();
  }

  fh.close();
  printMessage("---> Measurement end time: " + String(timeDiff));

  // turn blue LED off
  digitalWrite(LED_BUILTIN, HIGH);
}


String gyroDataToCsv(unsigned int id, unsigned long time, GyroData data) {


  String sep = ",";
  String result = id + sep + time + sep + 
    data.value.x_accel + sep + data.value.y_accel + sep +
    data.value.z_accel + "\n";
        
  return result;
}


void printMessage(String message) {
  sendUdpMessage(chipSerial + " " + message);
  Serial.println(chipSerial + " " + message);
}

// Calculate broadcast ip address
IPAddress calculateBroadcast(IPAddress ipaddress, IPAddress netmask) {
  IPAddress broadcast;
  broadcast[0] = ipaddress[0] | ( ~ netmask[0] );
  broadcast[1] = ipaddress[1] | ( ~ netmask[1] );
  broadcast[2] = ipaddress[2] | ( ~ netmask[2] );
  broadcast[3] = ipaddress[3] | ( ~ netmask[3] );
  return broadcast;
}


/********************************************************
                        FTP BLOCK
 *********************************************************/

byte sendFTP()
{
  fh = SPIFFS.open(String("/") + MEASUREMENT_FILENAME, FILE_READ);

  if(!fh) {
    Serial.println(F("File open fail."));
    return 0;    
  }
  
  if (client.connect(FTP_SERVER, FTP_PORT)) {
    Serial.println("FTP command port connected.");
  } 
  else {
    Serial.println("FTP command port connection failed.");
    fh.close();
    return 0;
  }

  if(!eRcv()) return 0; 
  client.write("USER test\r\n");
  if(!eRcv()) return 0;
  client.write("PASS test\r\n");
  if(!eRcv()) return 0;
  client.write("SYST\r\n");
  if(!eRcv()) return 0;
  client.write("PASV\r\n");
  if(!eRcv()) return 0;

  char *tStr = strtok(outBuf,"(,");
  int array_pasv[6];
  for ( int i = 0; i < 6; i++) {
    tStr = strtok(NULL,"(,");
    array_pasv[i] = atoi(tStr);
    if(tStr == NULL)
    {
      Serial.println(F("Bad PASV Answer"));    
    }
  }

  unsigned int hiPort,loPort;

  hiPort = array_pasv[4] << 8;
  loPort = array_pasv[5] & 255;

  hiPort = hiPort | loPort;
  Serial.print(F("FTP data port: "));
  Serial.println(hiPort);

  if (dclient.connect(FTP_SERVER, hiPort)) {
    Serial.println("FTP data port connected.");
  } 
  else {
    Serial.println("FTP data port connection failed.");
    client.stop();
    fh.close();
    return 0;
  }

  client.println(String("STOR ") + MEASUREMENT_FILENAME);

  if(!eRcv()) {
    dclient.stop();
    return 0;
  }

  byte clientBuf[64];
  int clientCount = 0;

  while(fh.available())
  {
    clientBuf[clientCount] = fh.read();
    clientCount++;

    if(clientCount > 63)
    {
      dclient.write(clientBuf, 64);
      clientCount = 0;
    }
  }

  if(clientCount > 0) dclient.write(clientBuf,clientCount);
  
  dclient.stop();
  if(!eRcv()) return 0;
  Serial.println(F("FTP data port disconnected."));
  
  client.write("QUIT\r\n");
  if(!eRcv()) return 0;

  dclient.stop();
  client.stop();
  Serial.println("FTP command port disconnected.");
  
  return 1;
}

byte eRcv()
{
  byte respCode;
  byte thisByte;

  while(!client.available()) delay(1);
  respCode = client.peek();
  outCount = 0;
  
  while(client.available()) {  
    thisByte = client.read();    
    Serial.write(thisByte);

    if(outCount < 127) {
      outBuf[outCount] = thisByte;
      outCount++;      
      outBuf[outCount] = 0;
    }
  }

  if(respCode >= '4') {
    efail();
    return 0;  
  }

  return 1;
}

void efail()
{
  byte thisByte = 0;

  client.write("QUIT\r\n");
  while(!client.available()) delay(1);
  while(client.available()) {  
    thisByte = client.read();    
    Serial.write(thisByte);
  }

  client.stop();
  dclient.stop();
  Serial.println("Something failed in FTP...disconnected.");
}
