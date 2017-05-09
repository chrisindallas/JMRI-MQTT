// JMRI_MQTT_mast_001
// mqttSignal12E.ino
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

#include <PubSubClient.h>
#include <ESP8266WiFi.h>


// This is still the DHCP version. Will add code to use static IP
// It subscribes to the mast.0001 topic and publishes to the callback topic

#define VERSION  "NodeMCU Signal, ESP8266-12E, 2017.05.09, 0.06"
#define BAUDRATE         115200
#define TIMEOUT_DELAY_MS   1000

// These definitions assume common anode connected LEDs
#define ON         0
#define OFF        1

// If you spend time at two locations?!?
#define OFFSITE 1
#include "wifiSettings.h"
// WifiSettings.h should have the following format:
//   #define MQTT_SERVER "192.168.1.100"
//   #define ssid ssid = "Your_SSID"
//   #define password = "Your_Password"

void callback( char* topic, byte* payload, unsigned int length );

const char* signalTopic = "mast.0001";
const char* callbackTopic = "callback";

// LEDs on ESP8266 GPIO
// These are the first three digital I/O on NodeMCU dev board: D0, D1 and D2
const int GRNLED = 4;
const int YELLED = 5;
const int REDLED = 16;

// Use names for states so it's easier to understand what the code is doing
const int S_CLEAR =    0;
const int S_APPROACH = 1;
const int S_STOP =     2;
const int S_UNLIT =    3;
const int S_UNUSED =   9;

static int state = S_STOP;           // state starts in STOP, but on MQTT connection, 
                                     // presistence will change the state

const long interval = 500;           // interval at which to blink ( milliseconds )

String clientName;

boolean BLINK2 = false;              // Should LED2 blink?
boolean ledState = false;

unsigned long previousMillis = 0;
unsigned long currentMillis  = 0;

WiFiClient wifiClient;
PubSubClient client( MQTT_SERVER, 1883, callback, wifiClient );

void setup( ) { 
  // initialize the light as an output and set to LOW ( off )
  pinMode( GRNLED, OUTPUT );
  pinMode( YELLED, OUTPUT );
  pinMode( REDLED, OUTPUT );
  allOn();
  
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
} // setup()

int loopsNoWifi = 0;

void loop( ) { 
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

  currentMillis = millis( );
  if ( BLINK2 ) { 
    if ( currentMillis - previousMillis >= interval ) { 
      // 1000ms have elapsed, save currentMillis for next blink
      previousMillis = currentMillis;
      // New state of LED is opposite of old state
      ledState ? ledState=ON : ledState=OFF;
      digitalWrite( YELLED, ledState );
    } // if
  } // if

  if ( Serial.available() > 1 ) {
    int inByte = Serial.read();
    if ( inByte == '?' ) {
      printState();
    } // if ?
    //Serial.println( inByte );
  } //
} // loop()


void allOn() {
  digitalWrite( GRNLED, ON );
  digitalWrite( YELLED, ON );
  digitalWrite( REDLED, ON );  
} // allOn()


void allOff() {
  digitalWrite( GRNLED, OFF );
  digitalWrite( YELLED, OFF );
  digitalWrite( REDLED, OFF );  
} // allOff()


void setStop() {
  BLINK2 = false;
  digitalWrite( GRNLED, OFF );
  digitalWrite( YELLED, OFF );
  digitalWrite( REDLED, ON );
} // setStop()


void setApproach() {
  BLINK2 = false;
  digitalWrite( GRNLED, OFF );
  digitalWrite( YELLED, ON );
  digitalWrite( REDLED, OFF );
} // setStop()


void printState() {
  Serial.print( signalTopic );
  Serial.print( " " );
  
  switch( state ) {
    case S_CLEAR:
      Serial.println( "CLEAR" );
      break;    
    case S_STOP:
      Serial.println( "STOP" );
      break;
    case S_APPROACH:
      Serial.println( "APPROACH" );
      break;
    case S_UNLIT:
      Serial.println( "UNLIT" );
      break;
    default:
      Serial.println( "NOT KNOWN" );     
  } // switch
} // printState()


void callback(  char* topic, byte* payload, unsigned int lengthPayload ) { 
  // convert topic to string to make it easier to work with
  String topicStr = topic; 

  // Print out some debugging info
  Serial.print( "\nTopic: " );
  Serial.println( topicStr );

  String myString = ( char* )payload;
  myString.remove( lengthPayload );
   Serial.print( "Received: " );
    Serial.print( myString );
  if ( myString == "Approach" ) { 
    state = S_APPROACH; 
    BLINK2 = true;
    digitalWrite( GRNLED, HIGH );
    digitalWrite( REDLED, HIGH );
    Serial.println( "...Approach set." );
  } else if ( myString == "Clear" ) {
    state = S_CLEAR;  
    BLINK2 = false;
    digitalWrite( GRNLED, ON );
    digitalWrite( YELLED, OFF );
    digitalWrite( REDLED, OFF );
    Serial.println( "...Clear set." );
  } else if ( myString == "Unlit" ) { 
    state = S_UNLIT; 
    BLINK2 = false;
    digitalWrite( GRNLED, OFF );
    digitalWrite( YELLED, OFF );
    digitalWrite( REDLED, OFF );
    Serial.println( "...Unlit set." );
  } else if ( myString == "Stop" ) { 
    state = S_STOP; 
    setStop();
    Serial.println( "...Stop set." );
  } else { 
    Serial.print( "Didn't recognize: " );
  } // if myString
  
  String combinedString = clientName + " " + myString;
  byte newPayload[ combinedString.length( ) + 1 ];
  combinedString.getBytes( newPayload,combinedString.length( ) + 1 );
  client.publish( callbackTopic, newPayload, combinedString.length( ) + 1 );
  Serial.println( combinedString );

} // callback(  char*, byte*, unsigned int )


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
      allOff();
      delay( 100 );
      setStop();
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
      clientName = "esp8266.12E-"+ String( WiFi.localIP( )[ 3 ] );    // +=

      Serial.print( clientName );
      Serial.print( " Attempting MQTT connection..." );
 
      // if connected, subscribe to the topic( s ) we want to be notified about
      if ( client.connect( ( char* ) clientName.c_str( ) ) ) { 
        Serial.print( "...MQTT Connected" );
        client.subscribe( signalTopic );
      } else {  // otherwise print failed for debugging 
        Serial.println( "...Failed." ); 
        //abort( ); 
        setApproach();
        delay( 100 );
        setStop();
        delay( TIMEOUT_DELAY_MS - 100 );        
      } // else
      if ( WiFi.status( ) != WL_CONNECTED ) {
        Serial.println( "...no WiFi either." );        
        break;   // get out, you won't connect without WiFi
      }
    } // while
  } // if
} // reconnect()

