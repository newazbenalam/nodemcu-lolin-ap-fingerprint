//Original source code : http://enrique.latorres.org/2017/10/17/testing-lolin-nodemcu-v3-esp8266/
//Download LoLin NodeMCU V3 ESP8266 Board for Arduino IDE (json) : http://arduino.esp8266.com/stable/package_esp8266com_index.json
#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>
//#include <ESP8266WebServer.h>
const char* ssid = "IoT_Vault";
const char* password = "12345678";
int ledPin = 2; // Arduino standard is GPIO13 but lolin nodeMCU is 2 http://www.esp8266.com/viewtopic.php?f=26&t=13410#p61332
int buzzer = D1;
WiFiServer server(80);
IPAddress IPaddr (192, 168, 0, 1);
IPAddress IPmask(255, 255, 255, 0);
SoftwareSerial mySerial(13, 15);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
//ESP8266WebServer server1(80);

int value = LOW;

String css = R"***(
<style>
*, *:before, *:after {
    box-sizing: inherit;
}
html, body {
    font-family: Verdana,sans-serif;
    font-size: 15px;
    line-height: 1.5;
}
p.padding {
    padding-left: 0.4cm;
}
p {
    color: black;
}
.w3-container, .w3-panel {
    padding: 0.01em 16px;
}
.w3-green, .w3-hover-green:hover {
    color: #fff!important;
    background-color: #4CAF50!important;
}
.w3-ripple {
    transition: opacity 0s;
}
.w3-btn, .w3-button {
    border: none;
    display: inline-block;
    padding: 8px 16px;
    vertical-align: middle;
    overflow: hidden;
    text-decoration: none;
    color: inherit;
    background-color: inherit;
    text-align: center;
    cursor: pointer;
    white-space: nowrap;
}
button, input, select, textarea, optgroup {
    font: inherit;
    margin: 0;
}
.w3-red, .w3-hover-red:hover {
    color: #fff!important;
    background-color: #f44336!important;
}=
</style>
)***";

void setup() {
 Serial.begin(115200);
 delay(10);
 pinMode(ledPin, OUTPUT);
 pinMode(buzzer, OUTPUT);
 digitalWrite(ledPin, LOW);
// Connect to WiFi network
Serial.println();
Serial.println();
 Serial.print("Connecting to ");
 Serial.println(ssid);
 WiFi.softAP(ssid, password);
 WiFi.softAPConfig(IPaddr, IPaddr, IPmask); 

// Start the server
 server.begin();
 Serial.println("Server started");

// Print the IP address
 Serial.print("Use this URL to connect: ");
 Serial.print("http://");
// Serial.print(WiFi.localIP());
 Serial.print("http://192.168.0.1");
 Serial.println("/");

// set the data rate for the sensor serial port
  finger.begin(57600);
  delay(5);
  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1) { delay(1); }
  }

  Serial.println(F("Reading sensor parameters"));
  finger.getParameters();
  Serial.print(F("Status: 0x")); Serial.println(finger.status_reg, HEX);
  Serial.print(F("Sys ID: 0x")); Serial.println(finger.system_id, HEX);
  Serial.print(F("Capacity: ")); Serial.println(finger.capacity);
  Serial.print(F("Security level: ")); Serial.println(finger.security_level);
  Serial.print(F("Device address: ")); Serial.println(finger.device_addr, HEX);
  Serial.print(F("Packet len: ")); Serial.println(finger.packet_len);
  Serial.print(F("Baud rate: ")); Serial.println(finger.baud_rate);

  finger.getTemplateCount();

  if (finger.templateCount == 0) {
    Serial.print("Sensor doesn't contain any fingerprint data. Please run the 'enroll' example.");
  }
  else {
    Serial.println("Waiting for valid finger...");
      Serial.print("Sensor contains "); Serial.print(finger.templateCount); Serial.println(" templates");
  }
}

void loop() {
 getFingerprintID();
 if (value == HIGH){
  delay(3000);
  digitalWrite(ledPin, LOW);
  value = LOW;
  }
 // Check if a client has connected
 WiFiClient client = server.available();
 if (!client) {
 return;
 }

// Wait until the client sends some data
 Serial.println("new client");
 while(!client.available()){
 delay(1);
 }

// Read the first line of the request
 String request = client.readStringUntil('r');
 Serial.println(request);
 client.flush();

//int value = LOW; //initially off
 if (request.indexOf("/LED=OFF") != -1) {
 digitalWrite(ledPin, HIGH);
 value = HIGH;
 digitalWrite(buzzer, LOW); // Turn the LED on
 delay(1000);  
 }
 if (request.indexOf("/LED=ON") != -1) {
 digitalWrite(ledPin, LOW);
 value = LOW;
 }

// Return the response
 client.println("HTTP/1.1 200 OK");
 client.println("Content-Type: text/html");
 client.println(""); 
 client.println("<!DOCTYPE HTML>");
 client.println("<html>");
 client.println("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
 client.println(css);
 client.println("<style>p.padding{padding-left: 0.4cm;}p{color: black;}cred{color: red}cgreen{color: green}</style>");

 client.print("<br><p class=\"padding\">Lock is : ");
 //High=off
 //Low=on

if(value == HIGH) {
 client.print("<cred>Off</cred>");
 } else {
 client.print("<cgreen>On<cgreen></p>");
 }
 client.println("<div class=\"w3-container\">");
 client.println("<br>");
 client.println("<a href=\"/LED=ON\"\"><button class=\"w3-btn w3-ripple w3-red\">Lock </button></a>");
 client.println("<a href=\"/LED=OFF\"\"><button class=\"w3-btn w3-ripple w3-green\">Unlock </button></a><br />");
 client.println("</div>");

 client.println("<div class=\"w3-container\">");
 client.println("<br>");
 client.println("<a href=\"/adduser\"\"><button class=\"w3-btn w3-ripple w3-green\">Add FingerPrint </button></a>");
 client.println("<a href=\"/resetuser\"\"><button class=\"w3-btn w3-ripple w3-red\">Reset Fingerprint </button></a><br />");
 client.println("</div>");
 
 client.println("</html>");

delay(1);
 Serial.println("Client disonnected");
 Serial.println("");
}

//Fingerprint code
uint8_t getFingerprintID() {
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println("No finger detected");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK success!

  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK converted!
  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK) {
    Serial.println("Found a print match!");
     digitalWrite(ledPin, HIGH); // Turn the LED on
     delay(5000);
     digitalWrite(ledPin, LOW);
     delay(1000);
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("Did not find a match");
    digitalWrite(buzzer, HIGH);
    delay(1000);
    digitalWrite(buzzer, LOW);
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }

  // found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID);
  Serial.print(" with confidence of "); Serial.println(finger.confidence);

  return finger.fingerID;
}

// returns -1 if failed, otherwise returns ID #
int getFingerprintIDez() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)  return -1;

  // found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID);
  Serial.print(" with confidence of "); Serial.println(finger.confidence);
  return finger.fingerID;
}
