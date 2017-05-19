// These definitions assume common anode connected LEDs
#define ON         0
#define OFF        1

#define NOTKNOWN 0
#define CLEAR    1
#define APPROACH 2
#define STOP     3
#define UNLIT    4

#define UPDATETIME 500

class Signal3pin {
  // Class Member Variables
  // These are initialized at startup
  int topPin;    // the red LED pin number
  int midPin;    // the yellow LED pin number
  int botPin;    // the green LED pin number

  int mastAspect = NOTKNOWN;
  int countDown;
  unsigned long previousMillis; 
  bool blink = false;

  public:
  // Constructor - creates a Signal3pin 
  // and initializes the member variables, state and pins
  Signal3pin( int rdPin, int ywPin, int gnPin ) { 
    topPin = rdPin; pinMode( topPin, OUTPUT ); 
    midPin = ywPin; pinMode( midPin, OUTPUT ); 
    botPin = gnPin; pinMode( botPin, OUTPUT );     
        
    mastAspect = NOTKNOWN; 
    Output();
    previousMillis = millis();
  } // Signal3pin(int, int) constructor

  void Clear() {
    mastAspect = CLEAR;
    Output();
  } // Clear()
  
  void Approach() {
    mastAspect = APPROACH;
    Output();
  } // Approach()
  
  void Stop() {
    mastAspect = STOP;
    Output();
  } // Stop()
  
  void Unlit() {
    mastAspect = UNLIT;
    Output();
  } // Unlit()
  
  void Reset() {
    mastAspect = NOTKNOWN;
    Output();
  } // Reset()
  
  // turn the led on for the current state, to avoid keeping track of which light to turn off, 
  //   we turn them all off, and then the needed one on.
  void Output() {        
    switch ( mastAspect ) {
      case NOTKNOWN:
        topAndBottom(); 
        break;                   // will do flash, red and green for now...
      case UNLIT:  // none
        unlit();
        break;
      case STOP :  // red
        bottom(); 
        break;     
      case APPROACH:  // yellow
        middle(); 
        break;     
      case CLEAR:  // green
        top(); 
        break;
      default: 
        break;
    }; // switch
  } // Output()

  // to avoid using "delay", we rather check the current time and so how far we are from
  //   last time, to decide if a lite needs to change color 
  void Update() {
    unsigned long now = millis();
     
    if ( now - previousMillis >= UPDATETIME ) {
      previousMillis = now;
      Output();                   // Update the actual LEDs
    } // if
  } // Update()
  
private:
  void top() {
    digitalWrite( topPin, ON );
    digitalWrite( midPin, OFF );
    digitalWrite( botPin, OFF );
  } // top()
  
  void middle() {
    digitalWrite( topPin, OFF );
    digitalWrite(midPin, !digitalRead(midPin));
    digitalWrite( botPin, OFF );    
  } // middle()
  
  void bottom() {
    digitalWrite( topPin, OFF );
    digitalWrite( midPin, OFF );
    digitalWrite( botPin, ON );    
  } // bottom()

  void unlit() {
    digitalWrite( topPin, OFF );
    digitalWrite( midPin, OFF );
    digitalWrite( botPin, OFF );    
  } // bottom()

  void topAndBottom() {
    digitalWrite( topPin, ON );
    digitalWrite( midPin, OFF );
    digitalWrite( botPin, ON );    
  } // topAndBottom()
  
}; // class Signal3pin

