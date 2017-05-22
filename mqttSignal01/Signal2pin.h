// Signal2pin.h
//
// Authors: Gert 'Speed' Muller
// 2017.05.21
//
// Controlling three seperate LEDs in a SignalMast with 2 pins.
// Using a software PWM to fade the previous aspect out and the news one in, by
// toggling between the two. Tick-Tock ends in Tock, so that is the last move.

#include <digitalWriteFast.h>

#define SIG_VERSION  "S2P, 2017.05.21, 0.03"

#define NOTKNOWN 0
#define CLEAR    1
#define APPROACH 2
#define STOP     3
#define UNLIT    4

#define TICK     0
#define TOCK     1

// 2 and 2 looks good!
#define UPDATETIME   2
// makes STEPS 0 for no fading, UPDATETIME can also go to 20 then
#define STEPS        2

class Signal2pin {
  // Class member variables, these are initialized at startup
  int topPin;    // the pin number between the yellow and red LED
  int botPin;    // the pin number between the yellow and green LED

  int ledState     = NOTKNOWN;  // boot up state
  int ledLastState = NOTKNOWN;  // when fading, toggle between current and lastState
  
  String verStr = SIG_VERSION;
  int width;
  int pulse;
  int ticktock;  // show the previous aspect on tick and the next (current) on tock

  unsigned long previousMillis; 

public:
  // Constructor - creates a Signal2pin 
  // and initializes the member variables, state and pins
  Signal2pin( int gyPin, int yrPin ) { 
    botPin = yrPin; pinMode( botPin, OUTPUT ); 
    topPin = gyPin; pinMode( topPin, OUTPUT );     
        
    ledState = NOTKNOWN; 
    ledLastState = ledState;
    width = STEPS;
    pulse = 0;
    ticktock = TOCK;      // start with the end when init.
    OutputTock( );
    previousMillis = millis( );
  } // Signal2pin( int, int ) constructor

  void Clear( ) {
    if ( ledState == CLEAR ) { // no change
      ;
    } else {
      startFadeOver( );
      ledState = CLEAR;
    } // else
  } // Clear( )
  
  void Approach( ) {
    if ( ledState == APPROACH ) { // no change
      ;
    } else {
      startFadeOver( );
      ledState = APPROACH;
    } // else
  } // Approach( )
  
  void Stop( ) {
    if ( ledState == STOP ) { // no change
      ;
    } else {
      startFadeOver( );
      ledState = STOP;
    } // else
  } // Stop( )
  
  void Unlit( ) {
    if ( ledState == UNLIT ) { // no change
      ;
    } else {
      startFadeOver( );
      ledState = UNLIT;
    } // else
  } // Unlit( )
  
  void Reset( ) {
    ledState = NOTKNOWN;
    OutputTock( );
  } // Reset( )
  
  // to avoid using "delay", we rather check the current time and so how far we are from
  //   last time, to decide if a lite needs to change color 
  //
  // since we can't turn all the pins off, we need fade from the last aspect to the new aspect
  void Update( ) {
    unsigned long now = millis( );
     
    if ( now - previousMillis >= UPDATETIME ) {
      previousMillis = now;
      if ( width <= STEPS ) {
        if ( pulse >= width ) {
          OutputTick( );
        } else {
          OutputTock( );
        } // if pulse
        if ( pulse++ > STEPS ) {
          pulse = 0;
          width++;
        } // if end of pulse 
      } else {
        OutputTock( );                   // Update actual
      } // time for change yet?
    } // if millis exceeds time
  } // Update( )

  // return the string version
  String version( ) {
    return verStr;
  } // version( )

  // these functions are only public since the WiFi and MQTT "reconnect" needs to change the LEDs
  // The fading requires "update( )" to be called, without "delays( )"!!! 
  void __top( ) {
    WRITE_PERI_REG( PERIPHS_GPIO_BASEADDR + 8, 1<<topPin ); //digitalWrite( topPin, LOW );
    WRITE_PERI_REG( PERIPHS_GPIO_BASEADDR + 8, 1<<botPin ); //digitalWrite( botPin, LOW );
  } // __top( )
  
  void __middle( ) {
    WRITE_PERI_REG( PERIPHS_GPIO_BASEADDR + 4, 1<<topPin ); //digitalWrite( topPin, HIGH );
    WRITE_PERI_REG( PERIPHS_GPIO_BASEADDR + 8, 1<<botPin ); //digitalWrite( botPin, LOW );    
  } // __middle( )
  
  void __bottom( ) {
    WRITE_PERI_REG( PERIPHS_GPIO_BASEADDR + 4, 1<<topPin ); //digitalWrite( topPin, HIGH );
    WRITE_PERI_REG( PERIPHS_GPIO_BASEADDR + 4, 1<<botPin ); //digitalWrite( botPin, HIGH );    
  } // __bottom( )

  void __topAndBottom( ) {
    WRITE_PERI_REG( PERIPHS_GPIO_BASEADDR + 8, 1<<topPin ); //digitalWrite( topPin, LOW );
    WRITE_PERI_REG( PERIPHS_GPIO_BASEADDR + 4, 1<<botPin ); //digitalWrite( botPin, HIGH );    
  } // __topAndBottom( )
    
private:
  // turn the leds on for the previous state
  void OutputTick( ) {        
    switch ( ledLastState ) {
      case NOTKNOWN:
        __topAndBottom( ); 
        break;                   // will do flash, red and green for now...
      case UNLIT:  // red
      case STOP :  // red
        __bottom( ); 
        break;     
      case APPROACH:  // yellow
        __middle( ); 
        break;     
      case CLEAR:  // green
        __top( ); 
        break;
      default: 
        break;
    }; // switch
  } // OutputTick( )
  
  // turn the leds on for the current state
  void OutputTock( ) {        
    switch ( ledState ) {
      case NOTKNOWN:
        __topAndBottom( ); 
        break;                   // will do flash, red and green for now...
      case UNLIT:  // red
      case STOP :  // red
        __bottom( ); 
        break;     
      case APPROACH:  // yellow
        __middle( ); 
        break;     
      case CLEAR:  // green
        __top( ); 
        break;
      default: 
        break;
    }; // switch
  } // OutputTock( )
  
  void startFadeOver( ) {
    ticktock = TICK;
    ledLastState = ledState;
    width = 0;
    pulse = 0;
  } // startFadeOver( )
    
}; // class Signal2pin

