// JMRI_MQTT_mast_001
// mqttSignal01.ino
//
// This code is for proof of concept of using the MQTT protocol,
// the ESP8266, mosquitto and JMRI.
// Uses the Arduino IDE to program the ESP8266.
//
// Authors: Chris Atkins, Gert 'Speed' Muller
// 2017.04.05, 2017.05.21
//
// Based on example code from ItKindaWorks - Creative Commons 2016
// github.com/ItKindaWorks
//
// Requires PubSubClient found here: https://github.com/knolleary/pubsubclient
//
// ESP8266 Simple MQTT signal controller

#include <PubSubClient.h>
#include <ESP8266WiFi.h>

// This is still the DHCP version. Will add code to use static IP
// It subscribes to the mast.0001 topic and publishes to the callback topic

#define VERSION  "NodeMCU Signal, ESP8266-01, 2017.05.12, 0.06"
#define BAUDRATE         115200
#define TIMEOUT_DELAY_MS   1000

// If you spend time at two locations?!?
#define OFFSITE 1
#include "wifiSettings.h"
// WifiSettings.h should have the following format:
//   #define MQTT_SERVER "192.168.1.100"
//   #define ssid ssid = "Your_SSID"
//   #define password = "Your_Password"

#include "Signal2pin.h"

void callback( char* topic, byte* payload, unsigned int length );

const char* signalTopic = "mast.0002";
const char* callbackTopic = "callback";

// LED pins on ESP8266-01 GPIO
const int LED1 = 0;
const int LED2 = 2;

const long interval = 500;           // interval at which to blink ( milliseconds )

String clientName;

int x = 0;
String str;
String address;

//boolean BLINK2 = false;

WiFiClient wifiClient;
PubSubClient client( MQTT_SERVER, 1883, callback, wifiClient );

Signal2pin signal( LED1, LED2 );

void setup( ) { 
  // initialize the signal
  signal.Reset();

  // start the serial line for debugging
  Serial.begin( BAUDRATE );
  delay( 100 );
  Serial.println( );
  Serial.println( VERSION );
  Serial.println( signal.version() );
  Serial.println( "STA mode" );    // avoid showing SSID AI-THINKER-xxxxxx
  WiFi.mode( WIFI_STA );  delay( 100 );

  //start wifi subsystem
  WiFi.begin( ssid, password );
  delay( 2000 );
  //attempt to connect to the WIFI network and then connect to the MQTT server
  reconnect( );

  //wait a bit before starting the main loop
  delay( 2000 );
  Serial.setTimeout( 2000 );
} // setup()

int loopsNoWifi = 0;

void loop( ) { 
  if( Serial.available() > 0 ) {
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

  signal.Update();  
} // loop


void callback(  char* topic, byte* payload, unsigned int lengthPayload  ) { 
  //convert topic to string to make it easier to work with
  String topicStr = topic; 

  //Print out some debugging info
  Serial.print( "Topic: " );
  Serial.println( topicStr );

  String myString = ( char* )payload;
  myString.remove( lengthPayload );

  if ( myString == "Approach" ) { 
    signal.Approach();
    Serial.print( "Expecting Approach, received: " );
  } else if ( myString == "Clear" ) { 
    signal.Clear();
    Serial.print( "Expecting Clear, received: " );
  } else if ( myString == "Unlit" ) { 
    signal.Unlit();
    Serial.print( "Expecting Unlit, received: " );
  } else if ( myString == "Stop" ) { 
    signal.Stop();
    Serial.print( "Expecting Stop, received: " );
  } else { 
    Serial.print( "Didn't recognize: " );
  } 
  Serial.println( myString );

  String combinedString = clientName + " " + myString;
  byte newPayload[ combinedString.length( )+1 ];
  combinedString.getBytes( newPayload,combinedString.length( )+1 );
  client.publish( callbackTopic, newPayload, combinedString.length( )+1 );
  Serial.println( combinedString );

} // callback


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
      signal.__middle();
      delay( 100 );
      signal.__bottom();
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
      clientName = "esp8266.01-"+ String( WiFi.localIP( )[ 3 ] );    // +=

      Serial.print( clientName );
      Serial.print( " Attempting MQTT connection..." );
 
      // if connected, subscribe to the topic( s ) we want to be notified about
      if ( client.connect( ( char* ) clientName.c_str( ) ) ) { 
        Serial.print( "...MQTT Connected" );
        client.subscribe( signalTopic );
      } else {  // otherwise print failed for debugging 
        Serial.println( "...Failed." ); 
        //abort( ); 
        signal.__top();
        delay( 100 );
        signal.__bottom();
        delay( TIMEOUT_DELAY_MS - 100 );        
      } // else
      if ( WiFi.status( ) != WL_CONNECTED ) {
        Serial.println( "...no WiFi either." );        
        break;   // get out, you won't connect without WiFi
      }
    } // while
  } // if
} // reconnect()
