
#define NOTKNOWN 0
#define CLEAR    1
#define APPROACH 2
#define STOP     3
#define UNLIT    4

#define UPDATETIME 500

class Signal2pin {
  // Class Member Variables
  // These are initialized at startup
  int topPin;    // the pin number between the yellow and red LED
  int botPin;    // the pin number between the yellow and green LED

  int ledState = NOTKNOWN;
  int countDown;
  unsigned long previousMillis; 

  public:
  // Constructor - creates a Signal2pin 
  // and initializes the member variables, state and pins
  Signal2pin( int gyPin, int yrPin ) { 
    botPin = yrPin; pinMode( botPin, OUTPUT ); 
    topPin = gyPin; pinMode( topPin, OUTPUT );     
        
    ledState = NOTKNOWN; 
    Output();
    previousMillis = millis();
  } // Signal2pin(int, int) constructor

  void Clear() {
    ledState = CLEAR;
    Output();
  } // Clear()
  
  void Approach() {
    ledState = APPROACH;
    Output();
  } // Approach()
  
  void Stop() {
    ledState = STOP;
    Output();
  } // Stop()
  
  void Unlit() {
    ledState = UNLIT;
    Output();
  } // Unlit()
  
  void Reset() {
    ledState = NOTKNOWN;
    Output();
  } // Reset()
  
  // turn the led on for the current state, to avoid keeping track of which light to turn off, 
  //   we turn them all off, and then the needed one on.
  void Output() {        
    switch ( ledState ) {
      case NOTKNOWN:
        topAndBottom(); 
        break;                   // will do flash, red and green for now...
      case UNLIT:  // red
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
      if ( countDown  > 0 ) {
        countDown--;
      } else {
        //ledState--;          // 4 goes to 3 only once, so no need to switch case 4:
        switch ( ledState ) {
          //case 0: ledState = 3;
          //case 3: countDown = GREEN_TIME;  break;
          //case 2: countDown = YELLOW_TIME; break;
          //case 1: countDown = RED_TIME;    break;      
          default: break;    
        }; // switch
        Output();                   // Update the actual LEDs
      } // time for change yet?
    } // if
  } // Update()
  
private:
  void top() {
    digitalWrite( topPin, LOW );
    digitalWrite( botPin, LOW );
  } // top()
  
  void middle() {
    digitalWrite( topPin, HIGH );
    digitalWrite( botPin, LOW );    
  } // middle()
  
  void bottom() {
    digitalWrite( topPin, HIGH );
    digitalWrite( botPin, HIGH );    
  } // bottom()

  void topAndBottom() {
    digitalWrite( topPin, LOW );
    digitalWrite( botPin, HIGH );    
  } // topAndBottom()
  
}; // class Signal2pin

