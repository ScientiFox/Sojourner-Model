/*
 * Original software for the Sojourner Model Controller
 * 
 * Provides Bluetooth wirless communication with the Rover, mode
 * control, and data relay to a computer.
 * 
 * Support libraries included in the Software package
 */
 
#include <SoftwareSerial.h> // Library for communication w/ Bluetooth

SoftwareSerial portOne(2, 3); // Serial port for BK communication

// Control and connection variables
int controlMode = 1; // Control mode (set by switch)
int connectionState = 0; // Active/dead connection variable

// Values read from the transmission
int batteryState = 0; // Battery level alert
int autonomousState = 0; // Autonomous navigation mode variable

// Sensor reading transmissions
int readingTemp;
int readingHumid;
int readingLight;

int LR; // Left/right control reading
int UD; // Up/down control reading

// Timers for synchronous operation
long contactTimer; // Timer marking packet reciept times
long transmitTimer; // Timer for transmissions
long printTimer; // Timer for data printouts to computer

// Flag marking send readiness
int sendUp = 1;

void setup() {

  portOne.begin(9600); // Serial port for BK communication

  Serial.begin(9600); // Serial port for data readout to computer

  // LED indicator pins
  pinMode(8,OUTPUT); // Control mode indicator
  pinMode(9,OUTPUT); // Active contact indicator

  // Battery Level indicators
  pinMode(10,OUTPUT);
  pinMode(11,OUTPUT);

  // Control mode switch input
  pinMode(12,INPUT_PULLUP);

  // Transmit notice of activity to computer via serial
  Serial.println("Controller Active");

}

void loop() {

  // Display state variables on controller LEDs
  digitalWrite(8,controlMode);
  digitalWrite(9,connectionState);
  digitalWrite(11,batteryState);
  digitalWrite(10,1-batteryState);

  // Read in control commands
  LR = analogRead(A2); // Left/right joystick axis read
  UD = analogRead(A3); // Up/down joystick axis read
  controlMode = digitalRead(12); // Control mode switch read

  if (connectionState != 1){
    // If control lost, not change to autonomous control
    controlMode = 0;}

  if (millis()-printTimer > 500){
    // 2x per second, print read data to computer serial port
    Serial.print("TP: ");
    Serial.print(readingTemp);
    Serial.print(" HM: ");
    Serial.print(readingHumid);
    Serial.print(" LL: ");
    Serial.print(readingLight);
    Serial.print(" BT: ");
    Serial.print(batteryState);
    Serial.print(" AS: ");
    Serial.print(autonomousState);
    Serial.println();

    // Reset timer for data out operations
    printTimer = millis();
  }

  if (millis() - contactTimer > 5000){
    // If 5 seconds elapsed time since receipt of last packet from
    //  rover, note loss of signal
    connectionState = 0;
    contactTimer = millis(); // Reset contact timer
    sendUp = 1; // Mark packet send flag- thus retrying for contact
                //  every 5 seconds
    }
  else{
    // If sufficiently frequent contact, mark active connection flag
    connectionState = 1;}

  if (portOne.available() > 5){
    // When a data packet is available
    while (portOne.read() != '&'){
      // Ignore partial packets
    }

    // Read in transmitted values
    readingTemp = portOne.parseInt();
    readingHumid = portOne.parseInt();
    readingLight = portOne.parseInt();
    batteryState = portOne.parseInt();
    autonomousState = portOne.parseInt();
    
    // Reset contact timer
    contactTimer = millis();

    // Empty buffer after full packet read
    while (portOne.available()){
      portOne.read();
    }

    // Mark packet send flag
    sendUp = 1;
  }

  if ((millis() - transmitTimer > 100)&&(sendUp == 1)){
    // With a frequency of at least 10/second, given ready to
    //  transmit flag is set, transmit control data to rover
    portOne.print("&");
    portOne.print(LR);
    portOne.print(",");
    portOne.print(UD);
    portOne.print(",");
    portOne.print(controlMode);
    portOne.print("$");

    // Reset transmission timer
    transmitTimer = millis();

    // Mark data as send
    sendUp = 0;
  }

}
