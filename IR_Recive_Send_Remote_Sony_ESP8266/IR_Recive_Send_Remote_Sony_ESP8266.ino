/*
 * IRremote with ESP8266 NodeMcu
 * Game room automation part #1
 * Netmedias https://www.youtube.com/channel/UCn8v7OzXk7IqRdKZdf14yjA
 * Developed by Nassir Malik
 */
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h> 
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <IRrecv.h>
#include <IRutils.h>

// @@@@@@@@@@@@@@@ You only need to midify modify wi-fi and domain info @@@@@@@@@@@@@@@@@@@@
const char* ssid     = "enter your ssid"; //enter your ssid/ wi-fi(case sensitive) router name - 2.4 Ghz only
const char* password = "enter ssid password";     // enter ssid password (case sensitive)
char host[] = "alexagoogleifttt.herokuapp.com"; //192.168.0.100 -enter your Heroku app name like  "alexagoogleifttt.herokuapp.com" 
// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
// NodeMcu pinout
IRsend irsend(4); //NodeMcu D2 = GPIO4
IRrecv irrecv(14); ////NodeMcu D5 = GPIO14
// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
decode_results results;
int port = 80;
char path[] = "/ws"; 
ESP8266WiFiMulti wifiMulti;
WebSocketsClient webSocket;
DynamicJsonBuffer jsonBuffer; 
String currState;
int pingCount = 0;
void sendCommand(int command){
  
  switch (command) {
    case 0:    // Turn on or off tv
      Serial.println("Turn on/Off TV");
      irsend.sendSony(0xa90,12,3);
      break;
    case 1:    // tv volume down
      Serial.println("Volum up");
      irsend.sendSony(0x490,12,3);
      delay(300);
      for (int i=0; i<12;i++){delay(10);irsend.sendSony(0x490,12,3);}
      break;
    case 2:    // tv volume down
      Serial.println("Volum Down");
      irsend.sendSony(0xc90,12,3);
      delay(300);
      for (int i=0; i<12;i++){delay(100);irsend.sendSony(0xc90,12,3);}
      break;
    case 3:    // your mute and unmute tv
      Serial.println("Mute");
      irsend.sendSony(0x290,12,3);
      break;
    case 4:    // 
      Serial.println("Launch HDMI input menu");
      irsend.sendSony(0xa50,12,3);;
      break;
    case 5:    // 
      Serial.println("Menu move up");
      irsend.sendSony(0x2f0,12,3);
      break;
    case 6:    // 
      Serial.println("Menu move down");
      irsend.sendSony(0xaf0,12,3);
      break;
    case 7:    // 
      Serial.println("Enter");
      irsend.sendSony(0xa70,12,3);
      break;
    case 8:    // 
      Serial.println("Exit tv menu");
      irsend.sendSony(0x62e9,12,3);
      break;
  }
  
  delay(1);        // delay in between commands
}


void setup()
{
    Serial.begin(115200);
    Serial.setDebugOutput(true);

      for(uint8_t t = 4; t > 0; t--) {
          delay(1000);
      }
    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    
    //Serial.println(ssid);
    wifiMulti.addAP(ssid, password);

    //WiFi.disconnect();
    while(wifiMulti.run() != WL_CONNECTED) {
      Serial.print(".");
        delay(100);
    }
    Serial.println("Connected to wi-fi");
    webSocket.begin(host, port, path);
    webSocket.onEvent(webSocketEvent);


  irsend.begin();
  delay(50);
  Serial.println("Enabling IRin");
  irrecv.enableIRIn(); // Start the receiver
  Serial.println("Enabled IRin");
}

void loop() {
  webSocket.loop();
  if (irrecv.decode(&results)) {
    Serial.print("Protocol: ");
    Serial.println(results.decode_type, DEC);
    serialPrintUint64(results.value, HEX);
    Serial.println("");
    irrecv.enableIRIn(); 
  delay(300); //5 second delay between each signal burst

  }
  delay(100);

    if (pingCount > 20) {
    pingCount = 0;
    webSocket.sendTXT("\"heartbeat\":\"keepalive\"");
  }
  else {
    pingCount += 1;
  }
}



void processWebScoketRequest(String data){

            JsonObject& root = jsonBuffer.parseObject(data);
            String device = (const char*)root["device"];
            String function = (const char*)root["function"];
            String value = (const char*)root["value"];
            String query = (const char*)root["query"];
            String message="";

            //Serial.println(value);
            if(query == "cmd"){ 
              Serial.println("Recieved command!");
              Serial.println(data);
                 sendCommand(value.toInt());
            }else if(query == "?"){ //if command then execute   
              Serial.println("For future use!");
            }else{//can not recognized the command
              Serial.println("Command is not recognized!");
            }
            Serial.print("Sending response back");
            Serial.println(message);
                  // send message to server
                  webSocket.sendTXT(message);
                  if(query == "cmd" || query == "?"){webSocket.sendTXT(message);}
}


void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) { //uint8_t *


    switch(type) {
        case WStype_DISCONNECTED:
           Serial.println("Disconnected! ");
           Serial.println("Connecting...");
               webSocket.begin(host, port, path);
               webSocket.onEvent(webSocketEvent);
            break;
            
        case WStype_CONNECTED:
            {
             Serial.println("Connected! ");
          // send message to server when Connected
            webSocket.sendTXT("Connected");
            }
            break;
            
        case WStype_TEXT:
            Serial.println("Got data");
              //data = (char*)payload;
           processWebScoketRequest((char*)payload);
            break;
            
        case WStype_BIN:

            //hexdump(payload, length);
            Serial.print("Got bin");
            // send data to server
            //webSocket.sendBIN(payload, length);
            break;
    }

}

