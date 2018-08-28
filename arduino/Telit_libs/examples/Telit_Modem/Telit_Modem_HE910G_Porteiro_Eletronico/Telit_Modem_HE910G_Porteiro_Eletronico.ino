/**
 * @file Telit_Modem_HE910G_Porteiro_Eletronico.ino
 *
 * HE910/UE910/UL865/UE866 telit modem		\n
 * EVK2 \n
 * Arduino M0
 *
 * @brief Exemplo porteiro eletrônico
 * 
 * Esse exemplo mostra como criar um sistema que permite ao acionar um botão a realização de uma ligação para um numero predefinido  
 *
 * @version 1.00 
 */
/*
 * INSTRUÇÕES DE UTILIZAÇÃO
 * 
 * Esse exemplo foi criado para utilização do kit EVK2 da Telit integrado com o arduino M0 PRO.
 * Para correto funcionamento a conexão das placas devem ser de acordo com a seguinte pinagem:
 *	Pino Arduino M0 Pro		  Pino EVK2
 *			GND					GND
 *			10					C104/RXD
 *			11					C103/TXD
 *			09					RESET
 * 
 * As demais conexões e configurações fica a escolha do usuario.
 * Para mais informações sobre o exemplo ou sobre a integração do Arduino e do kit da telit veja: https://www.????????????/
 */

#define SERIAL_BUFFER_SIZE 200

#include <XE910.h>

#include <Arduino.h>
#include "wiring_private.h"

#define RESP_MAX_LEN              300
#define BUTTON_PIN                  3
#define PHONE_NUMBER  "+5583-número do telefone-"

Uart serialModule (&sercom1, 11, 10, SERCOM_RX_PAD_0, UART_TX_PAD_2);
XE910 gsm(&serialModule);

void SERCOM1_Handler()
{
    serialModule.IrqHandler();
}

void setup() {
     
     Serial.begin(9600);
     serialModule.begin(115200);

     pinPeripheral(10, PIO_SERCOM);
     pinPeripheral(11, PIO_SERCOM);

     delay(1000);

     pinMode(BUTTON_PIN, INPUT);
          
     Serial.println("Testando aplicação Porteiro eletrônico...");
     
     delay(1000);
}

void loop() {
    
    
     if (gsm.ConfigureGSM()){
        
        int buttonState = LOW;

        Serial.println("Configure GSM: OK.");
        while(1){
            buttonState = digitalRead(BUTTON_PIN);
            if (buttonState == HIGH)
            {
                Serial.println("Doing a Call...");
                gsm.DoACall(PHONE_NUMBER);  
                delay(3000);
            }
            /*else{
                Serial.println("Closing a Call...");
                gsm.CloseCall();  
                delay(2000);
            }  
            */
        }  
        
    }
    while(!Serial.available()) { // Aguarda pressionar uma tecla.
        delay(100);
    }
    while(Serial.available()) {
        Serial.read();
        delay(100);
    }
    
}
