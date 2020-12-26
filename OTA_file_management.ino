#include <ESPAsyncWebServer.h>
#include <Update.h>
#include <ESPmDNS.h>
#include "SPIFFS.h"
#include <FS.h>
#define U_PART U_SPIFFS

const char* host = "esp32";
const char* ssid = "OPTUS_B5B9E8";
const char* password = "leadylaiks01462";
unsigned long timer1;
unsigned long timer2;
String filelist = "";
const char* PARAM = "file";

AsyncWebServer server(80);
size_t content_len;
File file;
bool opened = false;

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    html {
     font-family: Roboto, Arial, sans-serif;
     display: inline-block;
     margin: 0px auto;
     text-align: center;
    }
    h2 { font-size: 2.0rem; 
       font-family: Arial;
      text-align: center;
      font-weight: normal;
      color: #333333;
    }   
     .btn {
    background-color:rgba(0, 0, 0, 0);
    border: 1px solid #00acc1;
    box-sizing:border-box;
    outline-color:#00acc1;
    text-decoration:none;
      color: #00acc1;
      border-radius: 4px;
      padding: 9px 16px;
      background-image: none;
      letter-spacing:1.0px;
      cursor: pointer;
      text-transform:uppercase;
      font-size:14px;
      line-height: 18px;
      display: inline-block;
      vertical-align: middle;
     }
    .btn:hover {
   background-color:rgba(0, 172, 193, 0.04);
    }
    .btn:visited {
      color: #00acc1;
    }
     .btn:active {
   background-color:rgba(0, 172, 193, 0.09);
    }
  </style>
</head>
<body>
  <h2>OTA Firmware update & File management</h2>
    <title>Firmware update</title>
  <p>
    <a class="btn" href="/testpage">TESTPAGE</a>
    <a class="btn" href="/update">UPDATE</a>
  </p>
</body></html>)rawliteral";

const char upload_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html class="HTML">
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
<style>
input{background:#f1f1f1;border:0;padding:0 15px}
body{background:#00acc1;font-family:sans-serif;font-size:14px;color:#777}
#file-input,input{width: 100%;height:44px;border-radius:4px;margin:10px auto;font-size:15px}
#file-input{padding:0 10px;box-sizing:border-box;border:1px solid #ddd;line-height:44px;text-align:left;display:block;cursor:pointer; white-space: nowrap; overflow: hidden; text-overflow: ellipsis;}
#bar,#prgbar{background-color:#f1f1f1;border-radius:10px}#bar{background-color:#00acc1;width:0%;height:10px;}
form{background:#fff;max-width:258px;margin:75px auto;padding:35px; padding-bottom: 30px;border: 1px solid #00acc1;border-radius:5px;text-align:center}
.btn {background-color: #00acc1;box-shadow: 0 0 8px 0 rgba(0, 0, 0, 0.08), 0 0 15px 0 rgba(0, 0, 0, 0.02), 0 0 20px 4px rgba(0, 0, 0, 0.06);color: white;border-radius: 4px;padding: 9px 16px;background-image: none;
text-decoration: none;border: none;letter-spacing:1.25px;cursor: pointer;text-transform:uppercase;font-size:14px;line-height: 18px;display: inline-block;vertical-align: middle;}
.btn:hover {background-color: #0097a7;box-shadow: 0px 2px 4px -1px rgba(0, 0, 0, 0.2), 0px 4px 5px 0px rgba(0, 0, 0, 0.14), 0px 1px 10px 0px rgba(0,0,0,.12);}
.btn:visited {color: white;}
.btn:focus {outline: none;}
.btn:active {color: white;background-color: #007c91;}

.btn2 {background-color: #e29425;box-shadow: 0 0 8px 0 rgba(0, 0, 0, 0.08), 0 0 15px 0 rgba(0, 0, 0, 0.02), 0 0 20px 4px rgba(0, 0, 0, 0.06);color: white;border-radius: 4px;padding: 9px 16px;background-image: none;
text-decoration: none;border: none;letter-spacing:1.25px;cursor: pointer;text-transform:uppercase;font-size:14px;line-height: 18px;display: inline-block;vertical-align: middle;}
.btn2:hover:enabled {background-color: #d68a1d;box-shadow: 0px 2px 4px -1px rgba(0, 0, 0, 0.2), 0px 4px 5px 0px rgba(0, 0, 0, 0.14), 0px 1px 10px 0px rgba(0,0,0,.12);}
.btn2:visited {color: white;}
.btn2:focus {outline: none;}
.btn2:active {color: white;background-color: #ca7125;}
.btn2:disabled {
  cursor: default;
    background: #DDD;
}
   html {
     font-family: Roboto, Arial, sans-serif;
     display: inline-block;
     margin: 0px auto;
     text-align: center;
    }
    h2 { font-size: 3.0rem; 
       font-family: Arial;
      text-align: center;
      font-weight: normal;
      color: #fafcfc;
     }
    p { font-size: 3.0rem; margin-top: 0;}
    .units { font-size: 1.2rem; }
    .dht-labels{
      font-size: 1.5rem;
      vertical-align:middle;
      padding-bottom: 15px;
      font-weight: normal;
      color: #333333;
    }
</style>
</head>
    <title>FIRMWARE UPDATE</title>
   <script src='https://ajax.googleapis.com/ajax/libs/jquery/3.5.1/jquery.min.js'></script>
</head><body>
<form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>
<input type='file' name='update' id='file' onchange='sub(this)' style=display:none accept=".bin*">
<label id='file-input' for='file'>   Choose file...</label>
<input type="submit" id="updateBtn" class="btn2" disabled = "disabled" value="Update">

<input type="button" class="btn" onclick="location.href='/filesystem';" value="File manager">
<input type="button" class="btn" onclick="location.href='/';" value="Main page">
<br>
<div id='prg'style=display:none>Ready to Update</div>
<br><div id='prgbar'style=display:none><div id='bar'></div></div></form>
<script>
function sub(obj){
var a = obj.value;
console.log(a);
var fileName = a.replace(/^.*[\\\/]/, '')
console.log(fileName);
document.getElementById('file-input').innerHTML = fileName;
document.getElementById('updateBtn').disabled = false;
document.getElementById('prgbar').style.display = 'block';
document.getElementById('prg').style.display = 'block';
};
$('form').submit(function(e){
  document.getElementById('updateBtn').disabled = "disabled"; 
e.preventDefault();
var form = $('#upload_form')[0];
var data = new FormData(form);

$.ajax({
url: '/doUpdate',
type: 'POST',
data: data,
contentType: false,
processData:false,
xhr: function() {
var xhr = new window.XMLHttpRequest();
xhr.upload.addEventListener('progress', function(evt) {
if (evt.lengthComputable) {
var per = evt.loaded / evt.total;
$('#prg').html('Progress: ' + Math.round(per*100) + "%");
$('#bar').css('width',Math.round(per*100) + "%");
}
}, false);
return xhr;
},
success:function(d, s) {
console.log('success!'); 
alert("Successfully updated");
setTimeout("location.href = '../';", 2000);

},
error: function (a, b, c) {
}
});
});


</script></body></html>)rawliteral";

const char FS_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<!-- saved from url=(0031)http://192.168.0.160/filesystem -->
<html lang="en"><head><meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
<style>
input{background:#f1f1f1;border:0;padding:0 15px}
body{background:#00acc1;font-family:sans-serif;font-size:14px;color:#777}
#file-input,input{height:44px;border-radius:4px;margin:10px auto;font-size:15px}
#file-input{padding:0 10px;box-sizing:border-box;border:1px solid #ddd;line-height:44px;text-align:left;display:block;cursor:pointer; white-space: nowrap; overflow: hidden; text-overflow: ellipsis;}
#bar,#prgbar{background-color:#f1f1f1;border-radius:10px}#bar{background-color:#00acc1;width:0;height:10px;}
form{background:#fff;max-width:350px;margin:75px auto;padding:35px; padding-bottom: 30px;border: 1px solid #00acc1;border-radius:5px;text-align:center; align-content: center;}

.btn {background-color: #00acc1;box-shadow: 0 0 8px 0 rgba(0, 0, 0, 0.08), 0 0 15px 0 rgba(0, 0, 0, 0.02), 0 0 20px 4px rgba(0, 0, 0, 0.06);color: white;border-radius: 4px;padding: 9px 16px;background-image: none;
text-decoration: none;border: none;letter-spacing:1.25px;cursor: pointer;text-transform:uppercase;font-size:14px;line-height: 18px;display: inline-block;vertical-align: middle;transition-duration: 0.1s;}
.btn:hover {background-color: #0097a7;box-shadow: 0px 2px 4px -1px rgba(0, 0, 0, 0.2), 0px 4px 5px 0px rgba(0, 0, 0, 0.14), 0px 1px 10px 0px rgba(0,0,0,.12);}
.btn:visited {color: white;}
.btn:focus {outline: none;}
.btn:active {color: white;background-color: #007c91;}

.btn2 {background-color: #e29425;box-shadow: 0 0 8px 0 rgba(0, 0, 0, 0.08), 0 0 15px 0 rgba(0, 0, 0, 0.02), 0 0 20px 4px rgba(0, 0, 0, 0.06);color: white;border-radius: 4px;padding: 9px 16px;background-image: none;
text-decoration: none;border: none;letter-spacing:1.25px;cursor: pointer;text-transform:uppercase;font-size:14px;line-height: 18px;display: inline-block;vertical-align: middle;transition-duration: 0.1s;}
.btn2:hover:enabled {background-color: #d68a1d;box-shadow: 0px 2px 4px -1px rgba(0, 0, 0, 0.2), 0px 4px 5px 0px rgba(0, 0, 0, 0.14), 0px 1px 10px 0px rgba(0,0,0,.12);}
.btn2:visited {color: white;}
.btn2:focus {outline: none;}
.btn2:active {color: white;background-color: #ca7125;}
.btn2:disabled {
  cursor: default;
    background: #DDD;
}
.btndel {
  border: none;
  color: white;
  padding: 0px 6px;
  height: 25px;
  text-align: center;
  text-decoration: none;
  display: inline-block;
  font-size: 14px;
  margin: 2px 2px;
  cursor: pointer;
   border-radius: 4px;
   background-color: #e03423db;
   transition-duration: 0.1s;
  }
.btndel:hover {background-color: #b02f19;box-shadow: 0px 1px 3px -1px rgba(0, 0, 0, 0.2), 0px 4px 5px 0px rgba(0, 0, 0, 0.14), 0px 1px 10px 0px rgba(0,0,0,.12);}
.btndel:visited {color: white;}
.btndel:active {color: white;background-color: #db331d;}

   html {
     font-family: Roboto, Arial, sans-serif;
     display: inline-block;
     margin: 0px auto;
     text-align: center;
    }
    h2 { font-size: 3.0rem; 
       font-family: Arial;
      text-align: center;
      font-weight: normal;
      color: #fafcfc;
    }
    p { font-size: 3.0rem; margin-top: 0;}
    .units { font-size: 1.2rem; }
    .dht-labels{
      font-size: 1.5rem;
      vertical-align:middle;
      padding-bottom: 15px;
      font-weight: normal;
      color: #333333;
    }

table,th, td{
 font-family: arial;
  border-collapse: collapse;
  border: 1px solid #dddddd;


}

th, td {
  padding: 10px; 
}
tr:nth-child(odd) {
  background-color: #edebeb;
  
}

tr:nth-child(1) {
  background-color: #edebeb;
}

th:nth-child(2) {
  text-align: left;

}

/* nth-child(1) = the first td in each tr */
td:nth-child(1) {
 column-width: 30px;
 text-align: center;
  }

/* the second */
td:nth-child(2) {
  column-width: 175px;
  text-align: left;
  white-space: nowrap;
  overflow: hidden;
}
/* the third */
td:nth-child(3) {
 column-width: 100px;
 text-align: center;
 }

 td:nth-child(4) {
   column-width: 30px;
text-align: center;
 }
  
</style>

<title>File manager</title><script src='https://ajax.googleapis.com/ajax/libs/jquery/3.5.1/jquery.min.js'></script></head>
<body>
<form method="POST" action="http://192.168.0.160/filesystem#" enctype="multipart/form-data" id="upload_form">
<input type="file" name="update" id="file" onchange="sub(this)" style="display:none">
<label id="file-input" for="file">   Choose file...</label>
<input type="submit" id="updateBtn" class="btn2" disabled="true" value="Upload file">
<input type="button" class="btn" onclick="location.href=&#39;/update&#39;;" value="Back">
<br>
<h4 style="text-align: center;">File system content:</h4>
    <span id="filelist">%list%</span>
<br>
<div id="prg" style="display:none">Ready to upload</div>
<br><div id="prgbar" style="display:none"><div id="bar"></div></div></form>

<script>

function deletef(h) {
  
   var xhr = new XMLHttpRequest();
   var fnstring=String("/delete?file=")+h;
   xhr.open("GET", fnstring, true);
   console.log(fnstring);
    xhr.send();
    setTimeout("location.href = '../filesystem';", 3000);
}

function sub(obj){
  var a = obj.value;
  console.log(a);
  var fileName = a.replace(/^.*[\\\/]/, '');
  console.log(fileName);
  
  document.getElementById('file-input').innerHTML = fileName;
  
  document.getElementById('updateBtn').disabled = false;
  document.getElementById('prgbar').style.display = 'block';
  document.getElementById('prg').style.display = 'block';
};
$('form').submit(function(e){
document.getElementById('updateBtn').disabled = "disabled";  
e.preventDefault();
var form = $('#upload_form')[0];
var data = new FormData(form);

$.ajax({
url: '/doUpload',
type: 'POST',
data: data,
contentType: false,
processData:false,
xhr: function() {
var xhr = new window.XMLHttpRequest();
xhr.upload.addEventListener('progress', function(evt) {
if (evt.lengthComputable) {
var per = evt.loaded / evt.total;
$('#prg').html('Progress: ' + Math.round(per*100));
$('#bar').css('width', Math.round(per*350)+"px");
}
}, false);
return xhr;
},
success:function(d, s) {
console.log('success!'); 


setTimeout("location.href = '../filesystem';", 3000);

document.getElementById('prgbar').style.display = 'none';
document.getElementById('prg').style.display = 'none';
$('#prg').html('Ready to upload');
$('#bar').css('width:0px');
document.getElementById('file-input').innerHTML = 'Choose file...';
document.getElementById('updateBtn').disabled = true;

},
error: function (a, b, c) {
  alert("Upload error");
  setTimeout("location.href = '../filesystem';", 1000);
}
});
});

</script></body></html>)rawliteral";

String processor_update(const String& var) {
  Serial.println(var);
  if (var == "list") {
    return filelist;
  }
  return String();
}



void handleDoUpdate(AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final) {
  if (!index) {
    Serial.println("Update");
    content_len = request->contentLength();
    // if filename includes spiffs, update the spiffs partition
    int cmd = (filename.indexOf("spiffs") > -1) ? U_PART : U_FLASH;
    if (!Update.begin(UPDATE_SIZE_UNKNOWN, cmd)) {
      Update.printError(Serial);
    }
  }

  if (Update.write(data, len) != len) {
    Update.printError(Serial);
    Serial.printf("Progress: %d%%\n", (Update.progress() * 100) / Update.size());

  }

  if (final) {
    AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "Ok");
    response->addHeader("Refresh", "30");
    response->addHeader("Location", "/");
    request->send(response);
    if (!Update.end(true)) {
      Update.printError(Serial);
    } else {
      Serial.println("Update complete");
      Serial.flush();
      ESP.restart();
    }
  }
}

void handleDoUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
  if (!index) {
    content_len = request->contentLength();
    Serial.printf("UploadStart: %s\n", filename.c_str());
  }

  if (opened == false) {
    opened = true;
    file = SPIFFS.open(String("/") + filename, FILE_WRITE);
    if (!file) {
      Serial.println("- failed to open file for writing");
      return;
    }
  }

  if (file.write(data, len) != len) {
    Serial.println("- failed to write");
    return;
  }

  if (final) {

    AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "Ok");
    response->addHeader("Refresh", "20");
    response->addHeader("Location", "/filesystem");
    request->send(response);
    file.close();
    opened = false;
    Serial.println("---------------");
    Serial.println("Upload complete");

  }
}


void printProgress(size_t prg, size_t sz) {
  Serial.printf("Progress: %d%%\n", (prg * 100) / content_len);
}

void notFound(AsyncWebServerRequest *request) {
  if (request->url().startsWith("/")) {
    request->send(SPIFFS, request->url(), String(), true);
  } else {
    request->send(404);
  }
}


void listDir(fs::FS &fs, const char * dirname, uint8_t levels) {
  // filelist = "";
  int i = 0;
  String partlist;
  // Serial.printf("Listing directory: %s\r\n", dirname);
  File root = fs.open(dirname);
  if (!root) {
    Serial.println("- failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    Serial.println(" - not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file) {

    if (file.isDirectory()) {
      //  Serial.print("  DIR : ");
      // Serial.println(file.name());
      if (levels) {
        listDir(fs, file.name(), levels - 1);
      }
    } else {
      //  Serial.print("  FILE: ");
      //   Serial.print(file.name());
      //  Serial.print("\tSIZE: ");
      //   Serial.println(file.size());
      i++;
      String st_after_symb = String(file.name()).substring(String(file.name()).indexOf("/") + 1);

      partlist +=  String("<tr><td>") + String(i) + String("</td><td>") + String("<a href='") + String(file.name()) + String("'>") + st_after_symb + String("</td><td>") + String(file.size() / 1024) + String("</td><td>") + String("<input type='button' class='btndel' onclick=\"deletef('") + String(file.name()) + String("')\" value='X'>") + String("</td></tr>");
      filelist = String("<table><tbody><tr><th>#</th><th>File name</th><th>Size(KB)</th><th></th></tr>") + partlist + String(" </tbody></table>");
    }
    file = root.openNextFile();
  }
  filelist = String("<table><tbody><tr><th>#</th><th>File name</th><th>Size(KB)</th><th></th></tr>") + partlist + String(" </tbody></table>");
}

void deleteFile(fs::FS &fs, const String& path) {
  Serial.printf("Deleting file: %s\r\n", path);
  if (fs.remove(path)) {
    Serial.println("- file deleted");
  } else {
    Serial.println("- delete failed");
  }
}

void setup() {
  Serial.begin(115200);
  // Connect to WiFi network
  WiFi.begin(ssid, password);
  Serial.println("");

  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS Mount Failed");
    return;
  }

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
  if (!MDNS.begin(host)) { //http://esp32.local
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");
  timer1 = millis();
  timer2 = millis();
  listDir(SPIFFS, "/", 0);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/html", index_html);
  });

  server.on("/update", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/html", upload_html);
  });

  server.on("/filesystem", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/html", FS_HTML, processor_update);
  });

  server.on("/filelist", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/plain", filelist.c_str());
  });

  server.on("/testpage", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/testpage.html", String(), false);
  });

  server.on("/reboot", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain", "Device will reboot in 2 seconds");
    delay(2000);
    ESP.restart();
  });

  server.on("/doUpdate", HTTP_POST,
  [](AsyncWebServerRequest * request) {},
  [](AsyncWebServerRequest * request, const String & filename, size_t index, uint8_t *data, size_t len, bool final) {
    handleDoUpdate(request, filename, index, data, len, final);
  });

  server.on("/doUpload", HTTP_POST, [](AsyncWebServerRequest * request) {
    opened = false;
  },
  [](AsyncWebServerRequest * request, const String & filename, size_t index, uint8_t *data, size_t len, bool final) {
    handleDoUpload(request, filename, index, data, len, final);
  });

  server.on("/delete", HTTP_GET, [] (AsyncWebServerRequest * request) {
    String inputMessage;
    String inputParam;
    // GET input1 value on <ESP_IP>/update?state=<inputMessage>
    if (request->hasParam(PARAM)) {
      inputMessage = request->getParam(PARAM)->value();
      inputParam = PARAM;

      deleteFile(SPIFFS, inputMessage);

      Serial.println("-inputMessage-");
      Serial.print("File=");
      Serial.println(inputMessage);
      Serial.println(" has been deleted");

    }
    else {
      inputMessage = "No message sent";
      inputParam = "none";
    }
    request->send(200, "text/plain", "OK");
  });

  server.onNotFound(notFound);
  server.begin();
  Update.onProgress(printProgress);

}

void loop() {
  if (millis() - timer1 > 5000) {
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());


    timer1 = millis();
  }
  if (millis() - timer2 > 1000) {
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    listDir(SPIFFS, "/", 0);

    timer2 = millis();
  }


  delay(100);
}
