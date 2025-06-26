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

const char* ssid = "ESP32_SD_Manager";
const char* password = "12345678";

// HTML template with inline CSS and JS for upload progress
const char* headerHtml = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>ESP32 SD Manager</title>
  <style>
    body { font-family: Arial, sans-serif; max-width: 800px; margin: auto; padding: 10px; }
    table { width: 100%; border-collapse: collapse; margin-top: 10px; }
    th, td { padding: 8px; text-align: left; border-bottom: 1px solid #ddd; }
    th { background-color: #f4f4f4; }
    button { margin: 2px; padding: 5px 10px; }
    input[type=file] { margin-top: 10px; }
    progress { width: 100%; margin-top: 5px; }
    #uploadStatus { display: inline-block; margin-left: 10px; }
  </style>
  <script>
    function downloadFile(fname) {
      var xhr = new XMLHttpRequest();
      xhr.open('GET', '/download?file=' + encodeURIComponent(fname), true);
      xhr.responseType = 'blob';
      xhr.onprogress = function(event) {
        var percent = event.lengthComputable ? Math.floor((event.loaded / event.total) * 100) : '';
        document.getElementById('progress_' + fname).innerText = percent ? percent + '%' : '';
      };
      xhr.onload = function() {
        var url = window.URL.createObjectURL(xhr.response);
        var a = document.createElement('a');
        a.href = url;
        a.download = fname;
        a.click();
        window.URL.revokeObjectURL(url);
      };
      xhr.send();
    }
    function deleteFile(fname) {
      if (confirm('Delete ' + fname + '?')) {
        var xhr = new XMLHttpRequest();
        xhr.open('GET', '/delete?file=' + encodeURIComponent(fname), true);
        xhr.onload = function() { location.reload(); };
        xhr.send();
      }
    }
    function renameFile(fname) {
      var newname = prompt('Rename ' + fname + ' to:', fname);
      if (newname && newname !== fname) {
        var xhr = new XMLHttpRequest();
        xhr.open('GET', '/rename?file=' + encodeURIComponent(fname) + '&newname=' + encodeURIComponent(newname), true);
        xhr.onload = function() { location.reload(); };
        xhr.send();
      }
    }
    function uploadFile() {
      var fileInput = document.getElementById('fileInput');
      if (fileInput.files.length == 0) { alert('Select a file'); return; }
      var file = fileInput.files[0];
      var formData = new FormData();
      formData.append('upload', file);

      var xhr = new XMLHttpRequest();
      xhr.open('POST', '/upload', true);

      xhr.upload.onprogress = function(event) {
        var percent = event.lengthComputable ? Math.floor((event.loaded / event.total) * 100) : 0;
        document.getElementById('uploadProgress').value = percent;
        document.getElementById('uploadStatus').innerText = percent + '%';
      };
      xhr.onload = function() {
        if (xhr.status === 200) {
          document.getElementById('uploadStatus').innerText = 'Upload complete';
          setTimeout(function() { location.reload(); }, 1000);
        } else {
          document.getElementById('uploadStatus').innerText = 'Upload failed';
        }
      };
      xhr.send(formData);
    }
  </script>
</head>
<body>
  <h1>ESP32 SD File Manager</h1>
  <div>
    <input type="file" id="fileInput">
    <button onclick="uploadFile()">Upload</button>
    <progress id="uploadProgress" value="0" max="100"></progress>
    <span id="uploadStatus"></span>
  </div>
  <table>
    <tr><th>Name</th><th>Size (bytes)</th><th>Actions</th><th>Progress</th></tr>
)rawliteral";

const char* footerHtml = R"rawliteral(
  </table>
</body>
</html>
)rawliteral";

void listFiles() {
  String html = headerHtml;
  File root = SD.open("/");
  File file = root.openNextFile();
  while (file) {
    if (!file.isDirectory()) {
      String name = file.name();
      String size = String(file.size());
      html += "<tr>";
      html += "<td>" + name + "</td>";
      html += "<td>" + size + "</td>";
      html += "<td>";
      html += "<button onclick=\"downloadFile('" + name + "')\">Download</button>";
      html += "<button onclick=\"deleteFile('" + name + "')\">Delete</button>";
      html += "<button onclick=\"renameFile('" + name + "')\">Rename</button>";
      html += "</td>";
      html += "<td><span id='progress_" + name + "'></span></td>";
      html += "</tr>";
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
void loop() {
  server.handleClient();
}
