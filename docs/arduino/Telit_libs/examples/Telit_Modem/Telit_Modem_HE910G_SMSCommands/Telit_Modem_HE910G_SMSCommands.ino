/**
 * @file Telit_Modem_HE910G_SMSCommands.ino
 *
 * HE910/UE910/UL865/UE866 telit modem		\n
 * EVK2 \n
 * Arduino M0
 *
 * @brief Exemplo comandos por SMS
 * 
 * Este exemplo mostra como executar comandos através de Comunicação por SMS: 
 * Tasks ou rotinas que leem continuamente as mensagens SMS recebidas pelo módulo, 
 * lendo seu conteúdo e executando comandos conforme o texto enviado pela mensagem e com resposta(feedback) por SMS.
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
 * Para mais informações sobre o exemplo ou sobre a integração do Arduino e do kit da telit veja: https://www.????????????/
 * 
 */

#define SERIAL_BUFFER_SIZE 200

#include <XE910.h>

#include <Arduino.h>
#include "wiring_private.h"


#define RESP_MAX_LEN  200
#define RESET_PIN       9
#define LED_PIN        13

#define CELL_NUMBER               "+83-número do telefone-"
#define COMMAND_LED_ON            "led on"
#define COMMAND_LED_OFF           "led off"
#define COMMAND_LED_ON_RESP       "led on ok!"
#define COMMAND_LED_OFF_RESP      "led off ok!"

Uart serialModule (&sercom1, 11, 10, SERCOM_RX_PAD_0, UART_TX_PAD_2);
XE910 gsm(&serialModule);

void SERCOM1_Handler()
{
  serialModule.IrqHandler();
}

void setup()
{
     Serial.begin(9600);
     serialModule.begin(115200);

     pinPeripheral(10, PIO_SERCOM);
     pinPeripheral(11, PIO_SERCOM);

     delay(1000);
     
     pinMode(LED_PIN, OUTPUT);
     digitalWrite(LED_BUILTIN, LOW);
     
     Serial.println("Testando modulo GSM Message...");
};

void loop()
{
    Serial.println("Inicializando...");
       
    if ( gsm.ConfigureSMS() ) {
       
        char resp[RESP_MAX_LEN+1] = {0};
                      
        Serial.println("Cofiguração SMS: OK!");
        
        delay(10);

        Serial.println("Esperando comando por mensagem!");       
        while(1)
        {
            if(gsm.MessageRead(resp, 100))    
            {    
                if (!strcmp(resp,COMMAND_LED_ON))
                {
                    digitalWrite(LED_BUILTIN, HIGH);
                    Serial.println("Led ON!");
                    gsm.MessageWrite(CELL_NUMBER, COMMAND_LED_ON_RESP);
                }else if (!strcmp(resp,COMMAND_LED_OFF))
                {
                    digitalWrite(LED_BUILTIN, LOW);
                    Serial.println("Led OFF!");
                    gsm.MessageWrite(CELL_NUMBER, COMMAND_LED_OFF_RESP); 
                } 
            } 
        }
    }
    
    while(!Serial.available()) { // Aguarda pressionar uma tecla.
        delay(100);
    }
    while(Serial.available()) {
        Serial.read();
        delay(100);
    }
};


