/*!
 *  Print UVA index, UVB index and UV index on the serial monitor and a server
 */

// UVA/UVB Stuff
#include <Wire.h>
#include <DFRobot_VEML6075.h>
#define VEML6075_ADDR   0x10
DFRobot_VEML6075_IIC VEML6075(&Wire, VEML6075_ADDR);  // create object

// WiFi Stuff
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#ifndef STASSID
#define STASSID "network SSID"
#define STAPSK  "network password"
#endif

const char* ssid = STASSID;
const char* password = STAPSK;

ESP8266WebServer server(80);

const int led = 13;

// Functions
void handleRoot() {
  uint16_t    UvaRaw = VEML6075.readUvaRaw();         // read UVA raw
  uint16_t    UvbRaw = VEML6075.readUvbRaw();         // read UVB raw
  uint16_t    comp1Raw = VEML6075.readUvComp1Raw();   // read COMP1 raw
  uint16_t    comp2Raw = VEML6075.readUvComp2Raw();   // read COMP2 raw

  float       Uva = VEML6075.getUva();                // get UVA
  float       Uvb = VEML6075.getUvb();                // get UVB
  float       Uvi = VEML6075.getUvi(Uva, Uvb);        // get UV index

  //  Compile and send server UVA/UVB message and print to Serial Monitor
  String server_message = "UVI Information\n";
  Serial.println();
  Serial.println("======== UVA and UVB Report ========");
  server_message += "\n";
  Serial.print("UVA raw:    ");
  server_message += "UVA raw:    ";
  Serial.println(UvaRaw);
  server_message += UvaRaw;
  server_message += "\n";
  Serial.print("UVB raw:    ");
  server_message += "UVB raw:    ";
  Serial.println(UvbRaw);
  server_message += UvbRaw;
  server_message += "\n";
  Serial.print("COMP1 raw:  ");
  server_message += "COMP1 raw:  ";
  Serial.println(comp1Raw);
  server_message += comp1Raw;
  server_message += "\n";
  Serial.print("COMP2 raw:  ");
  server_message += "COMP2 raw:  ";
  Serial.println(comp2Raw);
  server_message += comp2Raw;
  server_message += "\n";
  Serial.print("UVA:        ");
  server_message += "UVA:        ";
  Serial.println(Uva, 2);
  server_message += Uva;
  server_message += "\n";
  Serial.print("UVB:        ");
  server_message += "UVB:        ";
  Serial.println(Uvb, 2);
  server_message += Uvb;
  server_message += "\n";
  Serial.print("UVIndex:    ");
  server_message += "UVIndex:    ";
  Serial.print(Uvi, 2);
  server_message += Uvi;
  if(Uvi < UVI_LOW) {
    Serial.println("  UVI low");
    server_message += "  UVI low\n";
  }
  else if(Uvi < UVI_MODERATE) {
    Serial.println("  UVI moderate");
    server_message += "  UVI moderate\n";
  }
  else if(Uvi < UVI_HIGH) {
    Serial.println("  UVI high");
    server_message += "  UVI high\n";
  }
  else if(Uvi < UVI_VERY_HIGH) {
    Serial.println("  UVI very high");
    server_message += "  UVI very high\n";
  }
  else {
    Serial.println("  UVI extreme");
    server_message += "  UVI extreme\n";
  }
  Serial.print("mw/cm^2:    ");
  server_message += "mw/cm^2:    ";
  Serial.println(Uvi2mwpcm2(Uvi), 2);
  server_message += Uvi2mwpcm2(Uvi);
  server_message += "\n";
  delay(1000);

  // Send to Server
  digitalWrite(led, 1);
  server.send(200, "text/plain", server_message);
  digitalWrite(led, 0);
}

void handleNotFound() {
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(led, 0);
}

void setup(void)
{
  // Shared config
  pinMode(led, OUTPUT);
  digitalWrite(led, 0);
  Serial.begin(115200);
  delay(2000);
  while(!Serial);

  // Wifi config
  WiFi.mode(WIFI_STA);
  WiFi.persistent(false);
  WiFi.disconnect(true);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);

  server.on("/inline", []() {
    server.send(200, "text/plain", "this works as well");
  });

  server.on("/gif", []() {
    static const uint8_t gif[] PROGMEM = {
      0x47, 0x49, 0x46, 0x38, 0x37, 0x61, 0x10, 0x00, 0x10, 0x00, 0x80, 0x01,
      0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x2c, 0x00, 0x00, 0x00, 0x00,
      0x10, 0x00, 0x10, 0x00, 0x00, 0x02, 0x19, 0x8c, 0x8f, 0xa9, 0xcb, 0x9d,
      0x00, 0x5f, 0x74, 0xb4, 0x56, 0xb0, 0xb0, 0xd2, 0xf2, 0x35, 0x1e, 0x4c,
      0x0c, 0x24, 0x5a, 0xe6, 0x89, 0xa6, 0x4d, 0x01, 0x00, 0x3b
    };
    char gif_colored[sizeof(gif)];
    memcpy_P(gif_colored, gif, sizeof(gif));
    // Set the background to a random set of colors
    gif_colored[16] = millis() % 256;
    gif_colored[17] = millis() % 256;
    gif_colored[18] = millis() % 256;
    server.send(200, "image/gif", gif_colored, sizeof(gif_colored));
  });

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");

   // UVA/UVB Stuff
  Serial.println();
  while(VEML6075.begin() != true) {
    Serial.println("VEML6075 begin faild");
    delay(2000);
  }
  Serial.println("VEML6075 begin successed");
}

void loop(void)
{
  server.handleClient();
  MDNS.update();
}
