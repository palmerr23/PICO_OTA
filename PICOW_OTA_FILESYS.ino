/* 
 * PI PICO W OTA web updater with modest LittleFS flash filesystem management  
 ** format FS 
 ** list dir
 ** upload file
 ** create file 
 ** edit file
 ** delete file
 *  MUST be compiled with a partition scheme with a fileSystem big enough to hold the update.
 *  When the program loads it deletes firmware.bin and otacommand.bin 
 *  Adapted from the OTAWebUpdater example and
 *  https://github.com/roberttidey/BaseSupport
 *
 *  Richard Palmer 2023 
 */

#define LITTLE_FS 1
#define SDFS_FS   2   // External SD cards not yet supported
#define FILESYSTYPE LITTLE_FS 

const char* host = "PICO-OTA";

#define FILEBUFSIZ 4096

#ifndef WM_PORTALTIMEOUT
  #define WM_PORTALTIMEOUT 180
#endif

#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <LEAmDNS.h>
//#include <Update.h>
#include <WiFiServer.h>
//#include <HTTPUpdateServer.h>

#define MYWIFI  // allow credentials to be stored in a file
#ifdef MYWIFI
  #include "mywifi.h"
#else
  const char* ssid = "MYSSID";
  const char* password = "MYPASSWD";
#endif

WebServer server(80);
bool fsFound = false;

#include "webpages.h"
#include "filecode.h"

void setup(void) 
{
  Serial.begin(115200);
  delay(3000);

  fsFound = initFS(false, false); // is an FS already in place?

  // Connect to WiFi network
  WiFi.begin(ssid, password);
  Serial.println("WiFi starting");
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  /*use mdns for host name resolution*/
  if (!MDNS.begin(host))  // http://PICOOTA.local
    Serial.println("Error setting up MDNS responder!");   
  else
    Serial.printf("mDNS responder started. Hotstname = http://%s.local\n", host);
   
  server.on("/", HTTP_GET, handleMain);
  
  // upload file to FS. Three callbacks
  server.on("/update", HTTP_POST, []() {
    //deleteOTAfiles();
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    rp2040.reboot();
  }, []() {    
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      WiFiUDP::stopAll();
      Serial.printf("Update: %s\n", upload.filename.c_str());
      FSInfo64 i;
      LittleFS.begin();
      LittleFS.info64(i);
      uint32_t maxSketchSpace = i.totalBytes - i.usedBytes;
      if (!Update.begin(maxSketchSpace)) { //start with max available size
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      /* flashing firmware to PICO*/
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
    }
  });
 
  server.on("/delete", HTTP_GET, handleFileDelete);
  server.on("/main", HTTP_GET, handleMain); // JSON format used by /edit
  // second callback handles file uploads at that location
  server.on("/edit", HTTP_POST, []()
    {server.send(200, "text/html", "<meta http-equiv='refresh' content='1;url=/main'>File uploaded. <a href=/main>Back to list</a>"); }, 
    handleFileUpload
    ); 
  server.onNotFound([](){if(!handleFileRead(server.uri())) server.send(404, "text/plain", "404 FileNotFound");});
 
  server.begin();
}

void loop(void) 
{
  server.handleClient();
  MDNS.update();
  delay(1);
}
