#include <WiFi.h>
#include <WebServer.h>
#include "SD.h"
#include "SPI.h"
#include "FS.h"

#define SD_MISO 2
#define SD_MOSI 15
#define SD_SCLK 14
#define SD_CS 13

SPIClass spiSD(HSPI);
WebServer server(80);
File uploadFile;

const char* ssid = "ESP32 SD Manager";
    }
    file = root.openNextFile();
  }
  html += footerHtml;
  server.send(200, "text/html", html);
}

void handleFileDownload() {
  if (!server.hasArg("file")) { server.send(400, "text/plain", "File not specified"); return; }
  String filename = server.arg("file");
  File downloadFile = SD.open("/" + filename);
  if (!downloadFile) { server.send(404, "text/plain", "File not found"); return; }
  server.sendHeader("Content-Disposition", "attachment; filename=\"" + filename + "\"");
  server.sendHeader("Content-Length", String(downloadFile.size()));
  server.streamFile(downloadFile, "application/octet-stream");
  downloadFile.close();
}

void handleFileDelete() {
  if (!server.hasArg("file")) { server.send(400, "text/plain", "File not specified"); return; }
  String filename = server.arg("file");
  server.send( SD.remove("/" + filename) ? 200 : 500, "text/plain", SD.exists("/" + filename) ? "Delete failed" : "OK" );
}

void handleFileRename() {
  if (!server.hasArg("file") || !server.hasArg("newname")) { server.send(400, "text/plain", "Missing parameters"); return; }
  String oldname = server.arg("file");
  String newname = server.arg("newname");
  server.send( SD.rename("/" + oldname, "/" + newname) ? 200 : 500, "text/plain", SD.exists("/" + newname) ? "OK" : "Rename failed" );
}

void handleFileUpload() {
  HTTPUpload& upload = server.upload();
  if (upload.status == UPLOAD_FILE_START) {
    String filename = upload.filename;
    if (!filename.startsWith("/")) filename = "/" + filename;
    uploadFile = SD.open(filename, FILE_WRITE);
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (uploadFile) uploadFile.write(upload.buf, upload.currentSize);
  } else if (upload.status == UPLOAD_FILE_END) {
    if (uploadFile) uploadFile.close();
  }
}

void setup() {
  Serial.begin(115200);
  spiSD.begin(SD_SCLK, SD_MISO, SD_MOSI, SD_CS);
  if (!SD.begin(SD_CS, spiSD)) { Serial.println("SD init failed"); return; }
  Serial.println("SD initialized");
  WiFi.softAP(ssid, password);
  Serial.print("AP IP: "); Serial.println(WiFi.softAPIP());
  server.on("/", listFiles);
  server.on("/download", handleFileDownload);
  server.on("/delete", handleFileDelete);
  server.on("/rename", handleFileRename);
  server.on("/upload", HTTP_POST, [](){ server.send(200, "text/plain", "OK"); }, handleFileUpload);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}
