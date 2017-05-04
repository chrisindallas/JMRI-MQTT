// JMRI_MQTT_mast_001.ino
//
// This code is for proof of concept of using the MQTT protocol, the ESP8266, mosquitto and JMRI.
// Uses the Arduino IDE to program the ESP8266.
//
// Authors:Chris Atkins, Gert 'Speed' Muller
// May 4, 2017 
//
// Based on example code from ItKindaWorks - Creative Commons 2016
// github.com/ItKindaWorks
//
// Requires PubSubClient found here: https://github.com/knolleary/pubsubclient
//
// ESP8266 Simple MQTT signal controller

#include <PubSubClient.h>
#include <ESP8266WiFi.h>

//These definitions assume common anode connected LEDs.
#define ON 0
#define OFF 1

#define WORK 1
#include "WifiSettings.h"
// WifiSettings.h should have the following format:
//   #define MQTT_SERVER "192.168.1.100"
//   #define ssid ssid = "Your_SSID"
//   #define password = "Your_Password"

void callback(char* topic, byte* payload, unsigned int length);

// signalTopic is unique for each ESP8266
const char* signalTopic = "mast.001";
const char* callbackTopic = "callback";


//LED on ESP8266 GPIO
//These are the first three digital I/O on NodeMCU dev board: D0, D1 and D2.
const int LED1 = 4;
const int LED2 = 5;
const int LED3 = 16;

const long interval = 500;           // interval at which to blink (milliseconds)

String clientName;

boolean BLINK2 = false;              // Should LED2 blink?
boolean ledState = false;

unsigned long previousMillis = 0;
unsigned long currentMillis = 0;

//Create a wifi client
WiFiClient wifiClient;

//Create a MQTT client and attach to wifi client using default port 1883 and 
//call the callback function when messages are received
PubSubClient client(MQTT_SERVER, 1883, callback, wifiClient);

void setup() {
  //initialize the LEDs as outputs and set to LOW
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  digitalWrite(LED1, ON);
  digitalWrite(LED2, ON);
  digitalWrite(LED3, ON);

  //start the serial line for debugging
  Serial.begin(115200);
  delay(100);


  //start wifi subsystem
  //This turns off the access point SSID from being broadcasted
  WiFi.mode(WIFI_STA);
  
  //Connect to wifi
  WiFi.begin(ssid, password);
  
  //attempt to connect to the WIFI network and then connect to the MQTT server
  reconnect();

  //wait a bit before starting the main loop
  delay(2000);
}

void loop(){

  //reconnect if connection is lost
  if (!client.connected() && WiFi.status() == 3) {reconnect();}

  //maintain MQTT connection
  client.loop();

  //MUST delay to allow ESP8266 WIFI functions to run
  delay(10); 

  currentMillis = millis();
  if(BLINK2) {
    if (currentMillis - previousMillis >= interval) {
      // 1000ms have elapsed, save currentMillis for next blink
      previousMillis = currentMillis;
      //New state of LED is opposite of old state.
      ledState ? ledState=ON : ledState=OFF;
      digitalWrite(LED2,ledState);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int lengthPayload) {

  //convert topic to string to make it easier to work with
  String topicStr = topic; 

  //Print out some debugging info
  Serial.print("Topic: ");
  Serial.println(topicStr);

  String myString = (char*)payload;
  myString.remove(lengthPayload);

  //This logic can be expanded to handle different aspect cases.
  if (myString == "Approach") {
    BLINK2 = true;
    digitalWrite(LED1, OFF);
    digitalWrite(LED3, OFF);
    Serial.print("Expecting Approach, received: ");
  } else if (myString == "Clear") {
    BLINK2 = false;
    digitalWrite(LED1, ON);
    digitalWrite(LED2, OFF);
    digitalWrite(LED3, OFF);
    Serial.print("Expecting Clear, received: ");
  } else if (myString == "Unlit") {
    BLINK2 = false;
    digitalWrite(LED1, OFF);
    digitalWrite(LED2, OFF);
    digitalWrite(LED3, OFF);
    Serial.print("Expecting Unlit, received: ");
  } else if (myString == "Stop") {
    BLINK2 = false;
    digitalWrite(LED1, OFF);
    digitalWrite(LED2, OFF);
    digitalWrite(LED3, ON);
    Serial.print("Expecting Stop, received: ");
  } else {
    Serial.print("Didn't recognize: ");
  } 
  Serial.println(myString);

  String combinedString = clientName + " " + myString;
  //cast the payload string to bytes to be published.
  byte newPayload[combinedString.length()+1];
  combinedString.getBytes(newPayload,combinedString.length()+1);
  //Publish the payload to the callback topic for debugging.
  client.publish(callbackTopic, newPayload, combinedString.length()+1);
  Serial.println(combinedString);

}

void reconnect() {
  //attempt to connect to the wifi if connection is lost
  if(WiFi.status() != WL_CONNECTED){
    //debug printing
    Serial.print("Connecting to ");
    Serial.println(ssid);

    //loop while we wait for connection
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }

    //print out some more debug once connected
    Serial.println("");
    Serial.println("WiFi connected");  
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  }

  //make sure we are connected to WIFI before attemping to reconnect to MQTT
  if(WiFi.status() == WL_CONNECTED){
  // Loop until we're reconnected to the MQTT server
    while (!client.connected()) {
      Serial.print("Attempting MQTT connection...");

      clientName += "esp8266-"+ String(WiFi.localIP()[3]);

      //if connected, subscribe to the topic(s) we want to be notified about
      if (client.connect((char*) clientName.c_str())) {
        Serial.print("\tMTQQ Connected");
        client.subscribe(signalTopic);
      }

      //otherwise print failed for debugging
      else{Serial.println("\tFailed."); abort();}
    }
  }
}
