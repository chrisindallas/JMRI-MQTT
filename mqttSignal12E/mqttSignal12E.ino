// JMRI_MQTT_mast_001
// mqttsignal12E.ino
//
// This code is for proof of concept of using the MQTT protocol,
// the ESP8266, mosquitto and JMRI.
// Uses the Arduino IDE to program the ESP8266.
//
// Authors: Chris Atkins, Gert 'Speed' Muller
// 2017.04.05
//
// Based on example code from ItKindaWorks - Creative Commons 2016
// github.com/ItKindaWorks
//
// Requires PubSubClient found here: https://github.com/knolleary/pubsubclient
//
// ESP8266 Simple MQTT signal controller
//
// 06: better WiFi recovery
// 07: solid yellow on dwarfs for Clear and Approach. TODO: remove all GRNLED from Dwarf, or better, go OOP.
// 08: added support for Signal3pin class
// 09: added support for two 3-LED signal masts controlled by two MQTT topics

#include <PubSubClient.h>
#include <ESP8266WiFi.h>


// This is still the DHCP version. Will add code to use static IP
// It subscribes to the mast.0001 topic and publishes to the callback topic

#define VERSION  "NodeMCU signal, ESP8266-12E, 2017.05.16, 0.09"
#define BAUDRATE         115200
#define TIMEOUT_DELAY_MS   1000

#include "wifiSettings.h"

// If you spend time at two locations?!?
#define OFFSITE 0


// WifiSettings.h should have the following format:
//   #define MQTT_SERVER "192.168.1.100"
//   #define ssid ssid = "Your_SSID"
//   #define password = "Your_Password"

#include "signal3pin.h"

void callback( char* topic, byte* payload, unsigned int length );

const char* signalTopic1 = "mast.001";
const char* signalTopic2 = "mast.002";
const char* callbackTopic = "callback";

//NodeMCU 1.0 pin configurations
#define D0 16
#define D1 5 // I2C Bus SCL (clock)
#define D2 4 // I2C Bus SDA (data)
#define D3 0 // Flash pin
#define D4 2 // TXD1 - On board LED
#define D5 14 // HSCK
#define D6 12 // HMISO 
#define D7 13 // HMOSI - TXD2
#define D8 15 // HCS - TXD2
#define D9 3 // RX - RXD0 (Serial console)
#define D10 1 // TX - TXD0 (Serial console)
#define D11 9 // SD2 - SDD2
#define D12 10 // SD3 - SDD3

// LEDs on ESP8266 GPIO
// These are the first three digital I/O on NodeMCU dev board: D0, D1 and D2
const int LED1 = D2;
const int LED2 = D1;
const int LED3 = D0;
const int LED4 = D5;
const int LED5 = D4;
const int LED6 = D3;

const long interval = 500;           // interval at which to blink ( milliseconds )

String clientName;

int x = 0;
String str;
String address;

WiFiClient wifiClient;
PubSubClient client( MQTT_SERVER, 1883, callback, wifiClient );

//signal6pin signal1( LED1, LED2, LED3, LED4, LED5, LED6 );
Signal3pin signal1( LED1, LED2, LED3);
Signal3pin signal2( LED4, LED5, LED6);

void setup( ) {
  // initialize the signal
  signal1.Reset();
  signal2.Reset();


  // start the serial line for debugging
  Serial.begin( BAUDRATE );
  delay( 100 );
  Serial.println( VERSION );
  Serial.println( "STA mode" );
  WiFi.mode( WIFI_STA );
  delay( 100 );

  // start wifi subsystem
  WiFi.begin( ssid, password );
  delay( 2000 );
  // attempt to connect to the WIFI network and then connect to the MQTT server
  reconnect( );

  // wait a bit before starting the main loop
  delay( 2000 );
  Serial.setTimeout( 2000 );
} // setup()

int loopsNoWifi = 0;

void loop( ) {
  if ( Serial.available() > 0 ) {
    str = Serial.readStringUntil( ':' );
    Serial.print( str );
    Serial.print( ":" );
    address = Serial.readStringUntil( ';' );
    Serial.print( address );
    Serial.println( ";" );
    x = address.toInt();
    if ( x > 0 ) {
      Serial.print( "Will soon subscribe to topic mast." );
      Serial.println( address );
    } else {
      Serial.println( "err: need a number!" );
    } // if address is a number
    Serial.readString( );
  } // if Serial.available()

  if ( WiFi.status( ) != 3 ) {   // this will kick in 10 seconds after WiFi died
    loopsNoWifi++;
    Serial.println( "!!! oops, Wifi=no !!!" );
    if ( loopsNoWifi == 5 ) {
      reconnect();
    }
    delay( 1000 );
  }
  // reconnect if connection is lost
  if ( !client.connected( ) && WiFi.status( ) == 3 ) {
    Serial.println( "!!! oops, Wifi=yes, MQTT=no !!!" );
    reconnect( );
  } // if

  // maintain MQTT connection
  client.loop( );

  // MUST delay to allow ESP8266 WIFI functions to run
  delay( 10 );

  signal1.Update();
  signal2.Update();

} // loop


void callback(  char* topic, byte* payload, unsigned int lengthPayload ) {
  //convert topic to string to make it easier to work with
  String topicStr = topic;

  // Print out some debugging info
  Serial.print( "Topic: " );
  Serial.println( topicStr );
  String myString = ( char* )payload;
  myString.remove( lengthPayload );

  if ( topicStr == signalTopic1 ) {
    Serial.print("Topic 1 received: ");
    setAspect( signal1 , myString ); 
  }  // signalTopic1
  if ( topicStr == signalTopic2 ) {
    Serial.print("Topic 2 received: ");
    setAspect( signal2 , myString ); 
  } // signalTopic2


  String combinedString = clientName + " " + myString;
  byte newPayload[ combinedString.length( ) + 1 ];
  combinedString.getBytes( newPayload, combinedString.length( ) + 1 );
  client.publish( callbackTopic, newPayload, combinedString.length( ) + 1 );
  Serial.println( combinedString );

} // callback(  char*, byte*, unsigned int )

// This has a bug in it that I was working on when I committed.
void setAspect( Signal3pin& mySignal, String& myTopic ) {

  Serial.print("Topic received: ");
  Serial.println( myTopic );
  if ( myTopic == "Approach" ) {
    mySignal.Approach();
  } else if ( myTopic == "Clear" ) {
    mySignal.Clear();
  } else if ( myTopic == "Unlit" ) {
    mySignal.Unlit();
  } else if ( myTopic == "Stop" ) {
    mySignal.Stop();
  } else {
    Serial.print( "Didn't recognize: " );
  }
  Serial.println( myTopic );
}  // signalTopic1



void reconnect( ) {
  int retry = 0;
  // attempt to connect to the wifi if connection is lost
  if ( WiFi.status( ) != WL_CONNECTED ) {
    // debug printing
    Serial.print( "Connecting to " );
    Serial.println( ssid );

    // loop while we wait for connection
    while ( WiFi.status( ) != WL_CONNECTED ) {
      retry++;
      signal1.Approach();
      delay( 100 );
      signal1.Stop();
      delay( TIMEOUT_DELAY_MS - 100 );
      if ( ( retry % 20 ) == 0 )
        Serial.print( ".\n" );
      else
        Serial.print( "." );
    } // while

    // print out some more debug once connected
    Serial.println( "" );
    Serial.println( "WiFi connected" );
    Serial.print( "IP address: " );
    Serial.println( WiFi.localIP( ) );
  } // if

  // make sure we are connected to WIFI before attemping to reconnect to MQTT
  if ( WiFi.status( ) == WL_CONNECTED ) {
    // Loop until we're reconnected to the MQTT server
    while ( !client.connected( ) ) {
      clientName = "esp8266.12E-" + String( WiFi.localIP( )[ 3 ] );   // +=

      Serial.print( clientName );
      Serial.print( " Attempting MQTT connection..." );

      // if connected, subscribe to the topic( s ) we want to be notified about
      if ( client.connect( ( char* ) clientName.c_str( ) ) ) {
        Serial.print( "...MQTT Connected" );
        client.subscribe( signalTopic1 );
        client.subscribe( signalTopic2 );
      } else {  // otherwise print failed for debugging
        Serial.println( "...Failed." );
        //abort( );
        signal1.Clear();
        delay( 100 );
        signal1.Stop();
        delay( TIMEOUT_DELAY_MS - 100 );
      } // else
      if ( WiFi.status( ) != WL_CONNECTED ) {
        Serial.println( "...no WiFi either." );
        break;   // get out, you won't connect without WiFi
      }
    } // while
  } // if
} // reconnect()
