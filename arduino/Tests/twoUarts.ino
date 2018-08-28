#include <Arduino.h>   // required before wiring_private.h
#include "wiring_private.h" // pinPeripheral() function
 
Uart Serial2 (&sercom1, 11, 10, SERCOM_RX_PAD_0, UART_TX_PAD_2);
void SERCOM1_Handler()
{
  Serial2.IrqHandler();
}
 
void setup() {
  Serial.begin(9600);
 
  //Serial2.begin(9600); //modulo wifi
  Serial2.begin(115200); //modulo gsm e bluetooth
  
  // Assign pins 10 & 11 SERCOM functionality
  pinPeripheral(10, PIO_SERCOM);
  pinPeripheral(11, PIO_SERCOM);
}
 
void loop() {
  
   // print out the value you read on mySerial wired in loopback:
  while (Serial.available()) {
    Serial2.write(Serial.read());
  }
  
  while (Serial2.available()) {
    Serial.write(Serial2.read());
  }
  
  //Serial.println();
  //mySerial.println();
  delay(10);        // delay in between reads for stability
}
