
String generateContentBoundary() {

  char *letters = "0123456789";
  String randNums;
  for(unsigned char i = 1; i <= 24; i++)
  {
      randNums = randNums + letters[random(0, 9)];
  }

  return "--------------------------" + randNums;
}

//void sendDataToApi(File file) {
//
//  WiFiClient client;
//  Serial.println("Sending data");
//  client.setNoDelay(true);
//  
//  String boundary = generateContentBoundary();
//
//  if (client.connect(API_SERVER, API_PORT)) {
//    
//    client.print("POST /api/test HTTP/1.1\r\n");
//    client.print("Content-Type: application/json\r\n");
//    client.print("User-Agent: ESP32/" + chipSerial + "\r\n");
//    client.print("Accept: */*\r\n");
//    client.print("Cache-Control: no-cache\r\n");
//    client.print("Host: " + String(API_SERVER) + ":" + String(API_PORT) + "\r\n");
//    client.print("accept-encoding: gzip, deflate\r\n");
//
//    uint16_t contentLength = data.length();
//
//    client.print("content-length: " + String(contentLength) + "\r\n");
//    client.print("Connection: keep-alive\r\n");
//    client.print("\r\n");
// 
//    client.write(file);
//  
//    delay(200);
//    client.stop();
//  }
//}


//void sendDataToApi() {
//
//  WiFiClient client;
//  Serial.println("Sending data");
//  client.setNoDelay(true);
//  
//  String boundary = generateContentBoundary();
//  File file = SPIFFS.open(MEASUREMENT_FILENAME, FILE_READ);
//  
//  if (client.connect(API_SERVER, API_PORT)) {
//
//    // Create HTTP header
//    client.print("POST /api/test HTTP/1.1\r\n");
//    client.print("Content-Type: multipart/form-data; boundary=" + boundary + "\r\n");
//    client.print("User-Agent: ESP32/" + chipSerial + "\r\n");
//    client.print("Accept: */*\r\n");
//    client.print("Cache-Control: no-cache\r\n");
//    client.print("Host: " + String(API_SERVER) + ":" + String(API_PORT) + "\r\n");
//    client.print("accept-encoding: gzip, deflate\r\n");
//    
//    String contentBegin = 
//                   "\r\n--" + boundary + "\r\n" +
//                   "Content-Disposition: form-data; name=\"datafile\"; filename=\"measurements.csv\"\r\n" +
//                   "Content-Type: text/plain\r\n\r\n";
//
//    String contentEnd = "--" + boundary + "--\r\n";
//    
//    uint16_t contentLength = contentBegin.length() + file.size() + contentEnd.length() - 1;
//    
//    client.print("content-length: " + String(contentLength) + "\r\n");
//    client.print("Connection: keep-alive\r\n");
//    client.print(contentBegin);
//
////    while(file.available()) {
////      while(file.position() < file.size()) {
////        String dataBuff = file.readStringUntil('\n');
////        client.print(dataBuff);
////        if(file.position() == file.size()) client.print("\r\n");
////        else client.print("\n");
////      }
////    }
//
//    client.write(file);
//    
//    client.write("\r\n");
//  
//    client.print(contentEnd);
//    client.stop();
//    file.close();
//  }
//}

