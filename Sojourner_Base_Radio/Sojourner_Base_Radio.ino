/*
Radio check test for the sojourner controller link
*/

#include "Ping.h" //US distance sensor
#include "QTR.h" //IR distance sensor
#include <VirtualWire.h> //wireless communication library

Ping front(4); //Attach US pin

int setPoint = 7; //US distance threshold

//Pins for bluetooth module
const int transmit_pin = 11;
const int receive_pin = A0;
const int transmit_en_pin = 3;

//Initialization
void setup() {

  Serial.begin(9600); //Start serial port

  // Motor Control Pins
  pinMode(12,INPUT_PULLUP);
  pinMode(A3,INPUT_PULLUP);

  // Motor drive Pins
  pinMode(10,OUTPUT);
  pinMode(9,OUTPUT);
  pinMode(6,OUTPUT);
  pinMode(5,OUTPUT);

  // Initialise the IO and ISR for wireless comms
  vw_set_tx_pin(transmit_pin);
  vw_set_rx_pin(receive_pin);
  vw_set_ptt_pin(transmit_en_pin);
  vw_set_ptt_inverted(true); // Required for DR3100
  vw_setup(2000);       // Bits per sec
  vw_rx_start();       // Start the receiver PLL running

}

long timer = millis(); //Set a primary timer
int count = 0; //An instance counter

//main loop
void loop() {

  //Every 0.2s...
  if (millis() - timer > 200){
    long readFront = front.getInches(); //Check the US distance
    int rightIR = analogRead(A2); //Check the IR distances
    int leftIR = analogRead(A3);
    
    //If an obstacle is detected in frony,
    if (readFront > setPoint){
      digitalWrite(9,LOW); //Shift the motor control to the avoid turn
      analogWrite(10,150);
      digitalWrite(5,LOW);
      analogWrite(6,150);
      setPoint = 15;} //Obstacle avoid hysteresis
    else { //Otherwise reset to seek mode
      digitalWrite(10,LOW);
      analogWrite(9,0);
      digitalWrite(6,LOW);
      analogWrite(5,180);
      setPoint = 7;} //reset hysteresis

    //set all controls to default
    digitalWrite(5,LOW);
    digitalWrite(6,LOW);
    digitalWrite(9,LOW);
    digitalWrite(10,LOW);

    timer = millis(); //Reset timer
  }

  //Test message elements
  char msg[7] = {'h','e','l','l','o',' ','#'};

  msg[6] = count; //Index into the message for tracking

  vw_send((uint8_t *)msg, 7); //Send the message
  vw_wait_tx(); // Wait until the whole message is gone

  //Increment counter
  count = count + 1;

  //Make a receipt buffer
  uint8_t buf[VW_MAX_MESSAGE_LEN];
  uint8_t buflen = VW_MAX_MESSAGE_LEN;
  delay(5); //Wait 5ms for the transmission to finish

  //Check if a message received
  if (vw_get_message(buf, &buflen)){
    Serial.print("Got: "); //If so, report it out
    for (int i = 0; i < buflen-1; i++){ //Read out char[]
      Serial.print(char(buf[i])); //print the char[] member
      Serial.print(' ');
    }
    Serial.print(buf[6]); //Report the last segment of the message separate
    Serial.println(' '); //Spacer and new line
  }
  
  // Motor disable switch
  while (digitalRead(12) == LOW){
    Serial.println(front.getInches());
    digitalWrite(5,LOW);
    digitalWrite(6,LOW);
    digitalWrite(9,LOW);
    digitalWrite(10,LOW);    
  }

  //Smoothness delay
  delay(100);
}
