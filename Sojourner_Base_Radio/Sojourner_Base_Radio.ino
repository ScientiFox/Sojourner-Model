#include "Ping.h"
#include "QTR.h"
#include <VirtualWire.h>

Ping front(4);

int setPoint = 7;

const int transmit_pin = 11;
const int receive_pin = A0;
const int transmit_en_pin = 3;

void setup() {

  Serial.begin(9600);

  // Motor Control Pin
  pinMode(12,INPUT_PULLUP);
  pinMode(A3,INPUT_PULLUP);

  // Motor Pins
  pinMode(10,OUTPUT);
  pinMode(9,OUTPUT);
  pinMode(6,OUTPUT);
  pinMode(5,OUTPUT);

  // Initialise the IO and ISR
  vw_set_tx_pin(transmit_pin);
  vw_set_rx_pin(receive_pin);
  vw_set_ptt_pin(transmit_en_pin);
  vw_set_ptt_inverted(true); // Required for DR3100
  vw_setup(2000);       // Bits per sec
  vw_rx_start();       // Start the receiver PLL running

}

long timer = millis();

int count = 0;

void loop() {

  if (millis() - timer > 200){
    long readFront = front.getInches();
    int rightIR = analogRead(A2);
    int leftIR = analogRead(A3);
    
    if (readFront > setPoint){
      digitalWrite(9,LOW);
      analogWrite(10,150);
      digitalWrite(5,LOW);
      analogWrite(6,150);
      setPoint = 15;}
    else {
      digitalWrite(10,LOW);
      analogWrite(9,0);
      digitalWrite(6,LOW);
      analogWrite(5,180);
      setPoint = 7;}

    digitalWrite(5,LOW);
    digitalWrite(6,LOW);
    digitalWrite(9,LOW);
    digitalWrite(10,LOW);

    timer = millis();
  }

  char msg[7] = {'h','e','l','l','o',' ','#'};
  msg[6] = count;
  vw_send((uint8_t *)msg, 7);
  vw_wait_tx(); // Wait until the whole message is gone
  count = count + 1;
  uint8_t buf[VW_MAX_MESSAGE_LEN];
  uint8_t buflen = VW_MAX_MESSAGE_LEN;
  delay(5);
  if (vw_get_message(buf, &buflen)){
    Serial.print("Got: ");
    for (int i = 0; i < buflen-1; i++){
      Serial.print(char(buf[i]));
      Serial.print(' ');
    }
    Serial.print(buf[6]);
    Serial.println(' ');
  }
  
  // Motor disable switch
  while (digitalRead(12) == LOW){
    Serial.println(front.getInches());
    digitalWrite(5,LOW);
    digitalWrite(6,LOW);
    digitalWrite(9,LOW);
    digitalWrite(10,LOW);    
  }

  delay(100);
}
