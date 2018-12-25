//V2018-12-11 08:20

// #include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPClient.h>
#include <WiFiUDP.h>

#include <FS.h>   // Include the SPIFFS library

#include <String.h>

#include <Wire.h>
#include <mpu6050.h>
#include <time.h>

#define SWAP(x,y) swap = x; x = y; y = swap

// Wifi Connection Data
//const char* ssid = "UPC1A97DF3-Bandi";
//const char* password = "Hpwp8enwtczm";
//char* ssid = "TD924570";
//char* password = "Qwedcxya";
char* ssid = "RA22SL";
char* password = "Sukoro70";

// HTTP Post connections
String http_data="http://bergulix.dyndns.org:8100/bow/web/m_data.php";
String http_imu="http://bergulix.dyndns.org:8100/bow/web/m_imu.php";

int NoOfMes = 500;

typedef union udpservermessage
{
  struct
  {
    char message[UDP_TX_PACKET_MAX_SIZE];
  } reg;
  struct
  {
    char instruction[10];
    char dt[10];
    char s1[1];
    char tm[8];
    char s2[1];
    char id[8];

  } value;
};

typedef union accel_t_gyro_union
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


String getContentType(String filename); // convert the file extension to the MIME type
bool handleFileRead(String path);       // send the right file to the client (if it exists)

int error;
uint8_t c;
double dT;
int count;
int icount;
uint8_t swap;

unsigned long time0;
unsigned long time1;
int i = 0;
int ChipId;
accel_t_gyro_union accel_t_gyro;

String HostName;
char HostNameC[40];

// Wifi & udp variables
boolean wifiConnected = false;
boolean udpConnected = false;
unsigned int udpPort = 37666;
IPAddress broadcast;
WiFiUDP UDP;
udpservermessage packetBuffer; //buffer to hold incoming packet,
char replyBuffer[200] = ""; // a string to send 
char instructionStart[10] = "Start    ";
String message;

void setup() {
  
  Serial.begin(115200);         // Start the Serial communication to send messages to the computer
  delay(10);
  Serial.println('\n');

  // Identify the HW address
  ChipId = ESP.getChipId();
  HostName =  String(ChipId, HEX);
  HostName.toCharArray(HostNameC, 10);
  Serial.print("Host Name: ");
  Serial.println(HostName);

  
  // Initialise wifi connection
  wifiConnected = connectWifi(ssid, password);
  if (wifiConnected) {
    Serial.print("Broadcast address: ");
    Serial.println(broadcast);
    udpConnected = connectUDP(udpPort);
    }
    
  // Report status as UDP message
  SendPacket(broadcast, udpPort, HostNameC, sizeof(HostNameC));


  // Start the mDNS responder for esp8266.local
  /*if (MDNS.begin(HostName.c_str())) {
    Serial.println("mDNS responder started");
  } else {
    Serial.println("Error setting up MDNS responder!");
  }
  */

 
}

void loop() {

int id=0;
if (wifiConnected) {
    if (udpConnected) {
      //int packetSize = UDP.parsePacket();
      if (ReadPacket(packetBuffer.reg.message)) {   
        //SendPacket(broadcast, udpPort, replyBuffer,sizeof(replyBuffer));
        if (strcmp(packetBuffer.value.instruction,instructionStart)==0){
          Serial.print("Message received full: ");
          Serial.println(packetBuffer.reg.message);  
          Serial.print("Instruction: ");
          Serial.print(packetBuffer.value.instruction);
          Serial.println("....");
          Serial.print("Date: ");
          Serial.println(packetBuffer.value.dt);
          Serial.print("Time: ");
          Serial.println(packetBuffer.value.tm);
          Serial.print("HeadID: ");
          Serial.println(packetBuffer.value.id);
          
          Serial.println("-----MERES-----");
       
          //Write mesurement header
          id=SendImu(packetBuffer.value.id, HostNameC);
          if (id>0) {
            String message = HostName+"---Measurement Start, id="+String(id);
            SendUdpMessage(message);

            SensorSetup();
            Mesurement(NoOfMes);
            SendData(id);
          } else {
            Serial.println("Imu head data upload failed");
          }
        Serial.println("--- WAITING FOR NEW START --- ");    
        message = HostName+"--- WAITING FOR NEW START ---";
        SendUdpMessage(message); 
        } 
      }
    }
  }
}

// connect to wifi – returns true if successful or false if not
boolean connectWifi(char* ssid, char* password) {
  boolean state = true;
  int i = 0;
  IPAddress iplocal;
  IPAddress ipnetmask;
  WiFi.begin(ssid, password);
  Serial.println("");
  Serial.println("Connecting to WiFi");
  IPAddress local;

  // Wait for connection
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (i > 40) {
      state = false;
      break;
    }
    i++;
  }
  if (state) {
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    iplocal=WiFi.localIP();
    ipnetmask=WiFi.subnetMask();
    broadcast[0] = iplocal[0] | ( ~ ipnetmask[0] );
    broadcast[1] = iplocal[1] | ( ~ ipnetmask[1] );
    broadcast[2] = iplocal[2] | ( ~ ipnetmask[2] );
    broadcast[3] = iplocal[3] | ( ~ ipnetmask[3] );    
  }
  else {
    Serial.println("");
    Serial.println("Connection failed.");
  }
  return state;
}

// connect to UDP – returns true if successful or false if not
boolean connectUDP(unsigned port) {
  boolean state = false;
  Serial.println("Connecting to UDP");
  if (UDP.begin(port) == 1) {
    Serial.println("Connection successful");
    state = true;
  }
  else {
    Serial.println("Connection failed");
  }
  return state;
}

void SendPacket(IPAddress address, int port, char* message, int sz) {
  UDP.beginPacket(address, port);
  UDP.write(message, sz);
  UDP.endPacket();
}

void SendUdpMessage(String message){
  int count=0;
  for (int i=0; i++; i<200){
    replyBuffer[i]=(char)0;
    };
  message.toCharArray(replyBuffer, 200);
  SendPacket(broadcast, udpPort, replyBuffer, sizeof(replyBuffer));
}

boolean ReadPacket(char* message) {
  boolean state = false;
  int packetSize = UDP.parsePacket();
  if (packetSize)
  {
    // read the packet into packetBufffer
    UDP.read(message, UDP_TX_PACKET_MAX_SIZE);
    state = true;
  }
  return state;
}

void SensorSetup(){
  String message;
  // Wake up sensor
  Wire.begin();
  Serial.println();
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
  error = MPU6050_read (MPU6050_CONFIG,(uint8_t *) &mpu_config, 1);
  Serial.println(error, BIN);
  error = MPU6050_read (MPU6050_ACCEL_CONFIG, (uint8_t *) &mpu_accel,  1);
  Serial.println(error, BIN);
  uint8_t mpu_config_dlpf = mpu_config & 0b00000111;
  uint8_t mpu_config_esyn = (mpu_config & 0b00111000) >> 3;
  Serial.println("              87654321");
  Serial.print("Config      : ");
  Serial.println(mpu_config, BIN);
  Serial.print("Accel setup : ");
  Serial.println(mpu_accel, BIN);
  Serial.print("Config dlpf : ");
  Serial.println(mpu_config_dlpf, DEC);
  Serial.print("Config esyn : ");
  Serial.println(mpu_config_esyn, DEC);
  MPU6050_write_reg (MPU6050_PWR_MGMT_1, 0);
  Serial.println(F(""));
  Serial.println(F("MPU-6050 - Reading start:"));
  
  message = HostName+"---Imu Init OK";
  SendUdpMessage(message);
}

void Mesurement(int NopOfMes){
  String message;
  // Start the SPI Flash Files System
  SPIFFS.begin();
    // Start new file, remove old
  SPIFFS.remove("/index.html");
  File fw = SPIFFS.open("/index.html", "w");
  if (!fw) {
    Serial.println("file write open failed");
  }
  
  // Measurement
  Wire.begin();
  time0 = millis();
  Serial.print("-------Start : ");
  Serial.println(time0, DEC);

  message = HostName+"---Measurement start time"+String(time0);
  SendUdpMessage(message);

  for (count = 0; count < NoOfMes; count++) {
    error = MPU6050_read (MPU6050_ACCEL_XOUT_H, (uint8_t *) &accel_t_gyro, 6);
    time1 = millis();
    SWAP (accel_t_gyro.reg.x_accel_h, accel_t_gyro.reg.x_accel_l);
    SWAP (accel_t_gyro.reg.y_accel_h, accel_t_gyro.reg.y_accel_l);
    SWAP (accel_t_gyro.reg.z_accel_h, accel_t_gyro.reg.z_accel_l);
    //    SWAP (accel_t_gyro.reg.x_gyro_h, accel_t_gyro.reg.x_gyro_l);
    //    SWAP (accel_t_gyro.reg.y_gyro_h, accel_t_gyro.reg.y_gyro_l);
    //    SWAP (accel_t_gyro.reg.z_gyro_h, accel_t_gyro.reg.z_gyro_l);
    // Write data to file
    fw.print(time1-time0, DEC);
    fw.print(F(","));
    fw.print(accel_t_gyro.value.x_accel, DEC);
    fw.print(F(","));
    fw.print(accel_t_gyro.value.y_accel, DEC);
    fw.print(F(","));
    fw.print(accel_t_gyro.value.z_accel, DEC);
    // fw.print(F(", "));
    // fw.print(accel_t_gyro.value.x_gyro, DEC);
    // fw.print(F(", "));
    // fw.print(accel_t_gyro.value.y_gyro, DEC);
    // fw.print(F(", "));
    // fw.print(accel_t_gyro.value.z_gyro, DEC);
    fw.print(F(";"));
    yield();
  }
  fw.close();
  time1 = millis();

  Serial.print("------ Meres vege ------ ");
  Serial.print("------ Ideje : ");
  Serial.print(time1 - time0, DEC);
  Serial.print("------ Freqvencia : ");
  Serial.print(count / (time1 - time0), DEC);
  Serial.println("");

  message = HostName+"---Measurement end time: "+String(time1-time0);
  SendUdpMessage(message);
};

void SendData(int id){
  String message;
  // Start the SPI Flash Files System
  SPIFFS.begin(); 
  //Create post requests
  File fw = SPIFFS.open("/index.html", "r");
  if (!fw) {
    Serial.println("file read open failed");
  }

  message = HostName+"---Data Upload Start";
  SendUdpMessage(message);

  String ids;
  ids=String(id);
  
  HTTPClient http;
  String post0 = "m_id="+ids+"&data=";
  String post;
  String flr;
  int httpCode;
  String response;

  http.begin(http_data);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  count = 1;
  while (fw.available()) {
    icount = 0;
    post = post0;
    while (fw.available() and icount < 10) {
      flr = fw.readStringUntil(';');
      post = post + flr + ";";
      icount++;
    }
    Serial.print(count);
    Serial.print(" -- post:");
    Serial.print(post.c_str());
    Serial.println("--");

    httpCode = http.POST(post.c_str());
    response = http.getString();
//    http.end();

    Serial.println();
    Serial.print("HTTP response code ");
    Serial.println(httpCode);
    // http.writeToStream(&Serial);
    Serial.print("HTTP response");
    Serial.println(response);

    message = HostName+"---Data Packet No: "+String(count)+" Http response: "+httpCode+" Script response: "+response;
    SendUdpMessage(message);

    count++;
  }
  http.end();
};

int SendImu(char* headID, char* host){
  Serial.print("--- IMU Post ---");
  HTTPClient http;
  String post;
  String flr;
  int httpCode;
  String response;

  post="data=";
  post = post+host+",00,"+headID; // a 00 -ba kerül majd az imu config bináris reprezentációja
  Serial.print("Post message:");
  Serial.println(post);
  http.begin(http_imu);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  httpCode = http.POST(post.c_str());
  response = http.getString();
  http.end();

  Serial.println();
  Serial.print("HTTP response code:");
  Serial.println(httpCode);
  Serial.print("HTTP response:");
  Serial.println(response);
  Serial.print("IMU ID:");
  Serial.println(response.substring(3,99).toInt());
  Serial.print("Database status:");
  Serial.println(response.substring(0,0));
  
  if (httpCode==200 and response.substring(0,2)=="OK") {
    return response.substring(3,99).toInt();
  } else {
    return -1;
  }
 
};
