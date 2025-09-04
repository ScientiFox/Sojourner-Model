/*
 * Original software for the Sojourner Model Rover
 * 
 * Provides Bluetooth wireless serial communication with the 
 * Controller, sensor readings, and autonomous and manual control.
 * 
 * Support libraries included in the Software package
 */

#include "Ping.h" // Library for the Ultrasonic distance sensor
#include "DHT.h" // Librart for the temp/humidity Sensor
#include <SoftwareSerial.h> // Library for communicating w/ Bluetooth

// Definitions used by DHT library
#define DHTPIN 12
#define DHTTYPE DHT22

SoftwareSerial portOne(3, 2); // Serial port for BK communication
Ping front(4); // Ultrasonic sensor on pin 4
DHT dht(DHTPIN, DHTTYPE); // Temp/Humidity sensor

// Autonomous mode state machine control variable
int autonomousState = -1;

// Control command variables
int LR = -1; // left/right turn level
int UD = -1; // forward/back drive level
int CM = -1; // Control mode: 0- autonomous; 1- manual
int CMPrev = -1; // Tracking variable for mode switching
int controlLost = 0; // Station loss variable
int newData = 1; // Flag for updated data readings to transmit
int IRthreshold = 55; // Threshold variable for binary IR level

// Suite of timers for synchronous functions
long navSenseTimer; // Navigation sensing timing (Ultrasonic, IR)
long senseTimer; // Data sensing timer (temp/humidity/light)
long contactTimer; // Remote control contact timer
long turnTimer; // Turn duration timer
long maneuverTimer; // State duration timer
long tempHumTimer; // DHT sensor latency timer

// Sensor reading variables
bool lfIR,rfIR; // yea/nay IR detection
int lfIRs,rfIRs; // Analog IR readings
int readFront; // Ultrasonic distance (in inches)

// Sensing report variables
int readingTemp;
int readingHumid;
int readingLight;
int batteryState;

// Floating point direct temp/humidity readings
float temp;
float hum;


void get_IR(){
  // Utility function to check the IR sensors. Loads both the analog
  // and digital IR variables

  // Analog IR readings: 1024- to make larger values correspond to
  //  larger distances
  lfIRs = 1024-analogRead(A0);
  rfIRs = 1024-analogRead(A1);

  // Converted binary state variables
  lfIR = (lfIRs)>IRthreshold;
  rfIR = (rfIRs)>IRthreshold;

}

int controlToSpeed(int LR, int UD, int side){
  // Utility function to convert control codes to motor speed
  //  factors
  if (side == 1){
    LR = 2 - LR; // Flip left/right value on alternating sides
  }
  // Ugly logic to convert to 8-degree movement
  return (UD != 1)*(1 + (LR > 0))*(1-UD) + (UD == 1)*2*(LR - 1);
}

void setMotors(int leftSet, int rightSet){
  // Set the speed of the motors to a given percentage of max power
  //  via PWM (open loop)

  // Map power from -100%-100% onto signal strength, different
  //  output levels roughly equalizes relative motor strengths
  int left = map(leftSet,-100,100,-200,200);
  int right = map(rightSet,-100,100,-250,250);

  // Set of differing controls adjusts the operation of the H-bridge
  //  motor controller, only one direction pin should be written to
  //  at any time.
  if (left > 0){
    // Left motor forward
    digitalWrite(9,LOW);
    analogWrite(10,left);}
  else if (left == 0){
    // Left motor off
    digitalWrite(9,LOW);
    digitalWrite(10,LOW);}
  else{
    // Left motor reverse
    analogWrite(9,-1*left);
    digitalWrite(10,LOW);}
  
  if (right > 0){
    // Right motor forward
    digitalWrite(5,LOW);
    analogWrite(6,right);}
  else if (right == 0){
    // Right motor off
    digitalWrite(5,LOW);
    digitalWrite(6,LOW);}
  else{
    // Right motor reverse
    analogWrite(5,-1*right);
    digitalWrite(6,LOW);}
}

void setup() {

  // Left motor Pins
  pinMode(10,OUTPUT);
  pinMode(9,OUTPUT);

  // Right motor pins
  pinMode(6,OUTPUT);
  pinMode(5,OUTPUT);

  // Light sensor pin
  pinMode(A3,INPUT_PULLUP);

  // DHT communication pin
  pinMode(7,INPUT);

  // BK communication port turned on
  portOne.begin(9600);

  // Initialize DHT sensor
  dht.begin();

}

void loop() {

  if (portOne.available() > 5){
    // If a data packet is available
    while (portOne.read() != '&'){
      // remove any partial packets
    }
    // Read in control variables
    LR = portOne.parseInt();
    UD = portOne.parseInt();
    CM = portOne.parseInt();

    // On receipt of a packet, reset lost contact timer, and 
    //  note that data is available
    contactTimer = millis();
    controlLost = 0;
    newData = 1;

    // Empty buffer after complete packet read
    while (portOne.available() > 0){
      portOne.read();}
  }

  if (CMPrev != CM){
    // When switching between control modes, stop the motors in 
    //  anticipation of a next command
    setMotors(0,0);

    // When moving from manual to automatic, reset autonomous state
    //  to the default
    if ((CMPrev == 1)&&(CM == 0)){
      autonomousState = 0;}

    // Update prior mode variable
    CMPrev = CM;
  }

  if (CM == 1){
    // In manual control mode

    // Convert transmitted LR & UD commands to scaled values
    //  in {0,1,2}
    int LRp = 2 - (LR > 341)*(LR < 682) - (LR >= 682)*2;
    int UDp = 2 - (UD > 341)*(UD < 682) - (UD >= 682)*2;

    // Set motor speeds with valuse given by 50x the calculated
    //  multiplier (calculated value mixes LR&UD values to skid
    //  steering)
    int leftMotor = 50*controlToSpeed(LRp, UDp, 0);
    int rightMotor = 50*controlToSpeed(LRp, UDp, 1);
    setMotors(leftMotor,rightMotor);
  }
  else if (CM == 0){
    // Autonomous navigation mode

    // Read the navigational sensors
    get_IR();
    readFront = front.getInches();
    // Ignore '0' readings on ultrasonic sensor:
    readFront = (readFront == 0)*1000 + readFront;

    if (controlLost == 1){
      // Indicates contact lost with controller, attempt to return
      autonomousState = 9; // state 9 is the start of the return
      controlLost = 0; // Only respond to loss of control once
    }

    // Switch operates the autonomous mode state machine
    switch (autonomousState){
      case (0):
        // Default forward movement state
        setMotors(75,75); // Foward at 75% power
        if ((readFront < 6)||(lfIR == 1)||(rfIR == 1)){
          // On encountering an obstacle, proced to avoidance
          //  subroutine in state 2
          autonomousState = 2;
          }
        break;

      case (1):
        // General 'halt' state
        setMotors(0,0);
        break;

      case (2):
        // Start of avoidance routine, selects direction to deflect
        //  based on bstacle disposition
        if ((readFront <= 6)&&(lfIR == 0)&&(rfIR == 0)){
          // Obstacle seen only by front ultrasonic
          autonomousState = 3;}
        else if ((readFront <= 6)&&(lfIR == 1)&&(rfIR == 0)){
          // Obstacle seen by ultrasonic and left IR
          autonomousState = 4;}
        else if ((readFront <= 6)&&(lfIR == 0)&&(rfIR == 1)){
          // Obstacle seen by ultrasonic and right IR
          autonomousState = 5;}
        else if ((readFront <= 6)&&(lfIR == 1)&&(rfIR == 1)){
          // Obstacle seen by all sensors
          autonomousState = 3;}
        else if ((readFront > 6)&&(lfIR == 1)&&(rfIR == 0)){
          // Obstacle seen by left IR only
          autonomousState = 6;}
        else if ((readFront > 6)&&(lfIR == 0)&&(rfIR == 1)){
          // Obstacle seen by right IR only
          autonomousState = 6;}
        else if ((readFront > 6)&&(lfIR == 0)&&(rfIR == 0)){
          // Obstacle seen by no sensors (somehow)
          autonomousState = 0;}
        else if ((readFront > 6)&&(lfIR == 1)&&(rfIR == 1)){
          // Obstacle seen by both IR, but not ultrasonic
          autonomousState = 3;}

        // Start the timer registering the duration of a maneuver
        maneuverTimer = millis();
        break;

      case (3):
        // Mode 3 avoidance, full reverse
        setMotors(-100,-100);
        if ((readFront > 6)&&(lfIR == 0)&&(rfIR == 0)){
          // If clear of obstacles, use a short turn to not drive
          //  directly back to them
          autonomousState = 6;
          // Reset maneuver timer, as state transitions
          maneuverTimer = millis();
        }
        // If more than 5 seconds have elapsed without evading
        //  the obstacles, then try a turn to escape
        if (millis() - maneuverTimer > 5000){
          autonomousState = 6;
          maneuverTimer = millis();}
        break;

      case (4):
        // Mode 4 avoidance, back up to right
        setMotors(-100,-75);
        
        if ((readFront > 6)&&(lfIR == 0)&&(rfIR == 0)){
          // If clear of obstacles, return to default movement
          autonomousState = 0;
          maneuverTimer = millis();
          }
        if (millis() - maneuverTimer > 5000){
          // Same 5 second breakaway as prior state
          autonomousState = 6;
          maneuverTimer = millis();
          }
        break;

      case (5):
        // State 5 avoidance routine, back up to left
        setMotors(-75,-100);
        
        if ((readFront > 6)&&(lfIR == 0)&&(rfIR == 0)){
          // As with previous state, return to default when
          //  clear of obstacles
          autonomousState = 0;
          maneuverTimer = millis();}
        if (millis() - maneuverTimer > 5000){
          // Same 5 second escape as last two
          autonomousState = 6;
          maneuverTimer = millis();}
        break;

      case (6):
        // Short back up and turn, for breakaway, and reorientation
        //  after direct reverse avoidance
        setMotors(-100,-100);
        if (millis()-maneuverTimer > 500){
          // 1/2 second reverse before turning
          autonomousState = 7;}
        break;
      
      case (7):
        //Short turn after short reverse
        setMotors(75,-75); // At 75% power
        if (millis()-maneuverTimer > 1000){
          // Spend 1 second turning, then return to default state
          autonomousState = 0;}
        break;
      
      // Broken contact return- turn ~180 degrees, then go to default
      //  autonomous navigation 
      case (9):
        // Set turn timer, and proceed to turn state
        turnTimer = millis();
        autonomousState = 10;
        break;

      case (10):
        // Turn state
        setMotors(-100,100); // 100% power for this one

        if ((lfIR == 1)||(readFront < 6)){
          // On encountering an obstacle to the turn, go to minor
          //  inset subroutine to avoid
          autonomousState = 11;
        }
        else if (millis() - turnTimer > 2500){
          // Turn ends after 2.5 seconds- count includes time in
          //  subroutine, as avoidance already alters 180 turn
          autonomousState = 0;
        }
        break;

      case (11):
        // Small avoidance for when a 180 degree 'return to control'
        //  turn is interrupted
        setMotors(-100,0);
        if ((lfIR == 0)&&(readFront > 6)){
          // Go back to turn when clear of obstacle
          autonomousState = 10;}
        break;      
    }
  }

  if (millis()-senseTimer > 100){
    // Block to take readings from environmental sensors (10/second)
    if (millis() - tempHumTimer > 1000){
      // Temp humidity sensor latency of ~1 second, so no need
      //  to bog controller down reading them
      hum = dht.readHumidity();
      temp = dht.readTemperature();
      tempHumTimer = millis();
    }

    // Convert temp/humidity readings to ints for transmission
    readingTemp = int(temp);
    readingHumid = int(hum);

    // Read the light sensor and low-battery level line
    readingLight = analogRead(A3);
    batteryState = digitalRead(7);

    // Transmit data readings and autonomous control state to
    //  the controller
    portOne.print("&");
    portOne.print(readingTemp);
    portOne.print(",");
    portOne.print(readingHumid);
    portOne.print(",");
    portOne.print(readingLight);
    portOne.print(",");
    portOne.print(batteryState);
    portOne.print(",");
    portOne.print(autonomousState);
    portOne.print("$");

    // Mark that data has been sent
    newData = 0;
    
    // Reset sensor timer
    senseTimer = millis();
  }

  if ((millis() - contactTimer > 1000)&(CM == 1)){
    // Stop movement if in manual control mode and no signal has
    //  been received for 1 second
    setMotors(0,0);
  }
  if ((millis() - contactTimer > 5000)&(CM == 1)){
    // After 5 seconds of dead air, mark loss of signal, and 
    // set control mode to autonomy
    controlLost = 1;
    CM = 0;
  }

}
