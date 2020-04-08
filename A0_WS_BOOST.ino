#include <WiFi.h>
#include <SPIFFS.h>
#include <ESPAsyncWebServer.h>
#include <WebSocketsServer.h>
#include <esp32_can.h>

uint8_t codes[4] = {5, 0xB, 0xC, 0xD};
int idx = 0;
uint32_t tick = 0;

// Constants
const char *ssid = "A0-BOOST";
const char *password =  "12345678";
const char *msg_toggle_led = "toggleLED";
const char *msg_get_led = "getLEDState";
const int dns_port = 53;
const int http_port = 80;
const int ws_port = 1337;
const int led_pin = 13;

// Globals
AsyncWebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(1337);
char msg_buf[10];
int led_state = 0;

/***********************************************************
   Functions
*/

// Callback: receiving any WebSocket message
void onWebSocketEvent(uint8_t client_num,
                      WStype_t type,
                      uint8_t * payload,
                      size_t length) {

  // Figure out the type of WebSocket event
  switch (type) {

    // Client has disconnected
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\n", client_num);
      break;

    // New client has connected
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(client_num);
        Serial.printf("[%u] Connection from ", client_num);
        Serial.println(ip.toString());
      }
      break;

    // Handle text messages from client
    case WStype_TEXT:

      // Print out raw message
      Serial.printf("[%u] Received text: %s\n", client_num, payload);

      // Toggle LED
      if ( strcmp((char *)payload, "toggleLED") == 0 ) {
        led_state = led_state ? 0 : 1;
        Serial.printf("Toggling LED to %u\n", led_state);
        digitalWrite(led_pin, led_state);

        // Report the state of the LED
        //      } else if ( strcmp((char *)payload, "getLEDState") == 0 ) {
        //        sprintf(msg_buf, "%d", led_state);
        //        Serial.printf("Sending to [%u]: %s\n", client_num, msg_buf);
        //        webSocket.sendTXT(client_num, msg_buf);

        //      } else if ( strcmp((char *)payload, "getLEDState") == 0 ) {
        //        sprintf(msg_buf, "%d", random(0,100));
        //        Serial.printf("Sending to [%u]: %s\n", client_num, msg_buf);
        //        webSocket.sendTXT(client_num, msg_buf);

        // Message not recognized
      } else {
        Serial.println("[%u] Message not recognized");
      }
      break;

    // For everything else: do nothing
    case WStype_BIN:
    case WStype_ERROR:
    case WStype_FRAGMENT_TEXT_START:
    case WStype_FRAGMENT_BIN_START:
    case WStype_FRAGMENT:
    case WStype_FRAGMENT_FIN:
    default:
      break;
  }
}

// Callback: send homepage
void onIndexRequest(AsyncWebServerRequest *request) {
  IPAddress remote_ip = request->client()->remoteIP();
  Serial.println("[" + remote_ip.toString() +
                 "] HTTP GET request of " + request->url());
  request->send(SPIFFS, "/index.html", "text/html");
}

// Callback: send style sheet
//void onCSSRequest(AsyncWebServerRequest *request) {
//  IPAddress remote_ip = request->client()->remoteIP();
//  Serial.println("[" + remote_ip.toString() +
//                 "] HTTP GET request of " + request->url());
//  request->send(SPIFFS, "/style.css", "text/css");
//}

// Callback: send 404 if requested file does not exist
void onPageNotFound(AsyncWebServerRequest *request) {
  IPAddress remote_ip = request->client()->remoteIP();
  Serial.println("[" + remote_ip.toString() +
                 "] HTTP GET request of " + request->url());
  request->send(404, "text/plain", "Not found");
}

/***********************************************************
   Main
*/

//---

int ledState = LOW;             // ledState used to set the LED
unsigned long previousMillis = 0;        // will store last time LED was updated
const long interval = 1000;           // interval at which to blink (milliseconds)

//---

void setup() {
  // Init LED and turn off
  pinMode(led_pin, OUTPUT);
  digitalWrite(led_pin, LOW);

  // Start Serial port
  Serial.begin(115200);

  // Make sure we can read the file system
  if ( !SPIFFS.begin()) {
    Serial.println("Error mounting SPIFFS");
    while (1);
  }

  // Start access point
  WiFi.softAP(ssid, password);

  // Print our IP address
  Serial.println();
  Serial.println("AP running");
  Serial.print("My IP address: ");
  Serial.println(WiFi.softAPIP());

  // On HTTP request for root, provide index.html file
  server.on("/", HTTP_GET, onIndexRequest);

  //  // On HTTP request for style sheet, provide style.css
  //  server.on("/style.css", HTTP_GET, onCSSRequest);

  server.on("/led.js", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/led.js", "text/javascript");
  });

  //  server.on("/index", HTTP_GET, [](AsyncWebServerRequest *request){
  //    request->send(SPIFFS, "/index.html", "text/html");
  //  });

  server.on("/gauge.min.js", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/gauge.min.js", "text/javascript");
  });

  server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/script.js", "text/javascript");
  });

  server.on("/bootstrap.bundle.min.js", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/bootstrap.bundle.min.js", "text/javascript");
  });

  server.on("/jquery-3.3.1.min.js", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/jquery-3.3.1.min.js", "text/javascript");
  });

  server.on("/bootstrap.min.css", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/bootstrap.min.css", "text/css");
  });

  // Handle requests for pages that do not exist
  server.onNotFound(onPageNotFound);

  // Start web server
  server.begin();

  // Start WebSocket server and assign callback
  webSocket.begin();
  webSocket.onEvent(onWebSocketEvent);

  CAN0.setCANPins(GPIO_NUM_4, GPIO_NUM_5);
  CAN0.begin(500000);
  //Serial.println("Ready ...!");
  int filter;
  //extended
  for (filter = 0; filter < 3; filter++) {
    Can0.setRXFilter(filter, 0, 0, false);
    Can1.setRXFilter(filter, 0, 0, false);
  }
}

void loop() {

  CAN_FRAME incoming;

  if (Can0.available() > 0) {
    Can0.read(incoming);
    if (incoming.id > 0x7DF && incoming.id < 0x7F0)
    {
      processPID(incoming);
    }
  }

  static unsigned long l = 0;
  unsigned long t = millis();
  if ((t - l) > 50) {

    sendPIDRequest(0x7DF, 0xB);

    //sprintf(msg_buf, "%d", random(-60, 60));
    // webSocket.broadcastTXT(msg_buf);
    l = t;
  }
  webSocket.loop();
}

void sendPIDRequest(uint32_t id, uint8_t PID)
{
  CAN_FRAME frame;
  frame.id = id;
  frame.extended = 0;
  frame.length = 8;
  for (int i = 0; i < 8; i++) frame.data.bytes[i] = 0xAA;
  frame.data.bytes[0] = 2; //2 more bytes to follow
  frame.data.bytes[1] = 1;
  frame.data.bytes[2] = PID;
  Can0.sendFrame(frame);
}

void processPID(CAN_FRAME &frame)
{
  int temperature;
  float psi;
  int RPM;
  int MPH;

  if (frame.data.bytes[1] != 0x41) return; //not anything we're interested in then
  switch (frame.data.bytes[2])
  {
    //    case 5:
    //      temperature = frame.data.bytes[3] - 40;
    //      Serial.print("Coolant temperature (C): ");
    //      Serial.println(temperature);
    //      break;
    case 0xB:
      psi = frame.data.bytes[3] * 0.145038; //kPA to PSI
      psi = psi - 14.8;
      //psi = psi * 100;
      Serial.print("Manifold abs pressure (psi): ");
      Serial.println(psi);
      //psi= psi*3;
      //sprintf(msg_buf, "%d", psi);
      dtostrf(psi, 3, 2, msg_buf);
      webSocket.broadcastTXT(msg_buf);
      break;
    case 0xC:
      RPM = ((frame.data.bytes[3] * 256) + frame.data.bytes[4]) / 4;
      Serial.print("Engine RPM: ");
      Serial.println(RPM);
      sprintf(msg_buf, "%d", RPM);
      webSocket.broadcastTXT(msg_buf);
      break;
      //    case 0xD:
      //      MPH = frame.data.bytes[3] * 0.621371;
      //      Serial.print("Vehicle Speed (MPH): ");
      //      Serial.println(MPH);
      //      break;
  }
}
