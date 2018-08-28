/**
 * @file Telit_Modem_HE910G_Logger.ino
 *
 * HE910/UE910/UL865/UE866 telit modem		\n
 * EVK2 \n
 * Arduino M0
 *
 * @brief Exemplo logger GSM
 * 
 * Este é um exemplo de um sistema de logger baseado em modem de Loop infinito (Arduino) 
 * que repassa o tráfego de uma das portas seriais para um socket aberto com um servidor remoto e 
 * bufferiza os bytes que são recebidos enquanto a conexão não estava ativa. 
 * O sistema verifica que a conexão caiu (contexto com a APN ou o socket) e tenta continuamente uma reconexão. 
 * Permite também o fluxo de dados servidor->dispositivo, refletido na porta serial. Um tunel de porta serial por TCP.
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
 * As configurações de servidor, baudrate e afins deverão ser realizadas manualmente, através das constantes no código.
 * Para mais informações sobre o exemplo ou sobre a integração do Arduino e do kit da telit veja: https://www.????????????/
 */


#define SERIAL_BUFFER_SIZE 200

#include <XE910.h>

#include <Arduino.h>
#include "wiring_private.h"


#define RESP_MAX_LEN  200
#define RESET_PIN       9

#define GPRS_OI         "gprs.oi.com.br"
#define GPRS_PROTOCOL   "IP"

#define SERVER          "lieno.ddns.net"
#define PORT            9000


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
     
     Serial.println("Testando modulo GSM...");
};

void loop()
{
    Serial.println("Inicializando...");

    if ( gsm.Init(GPRS_OI,GPRS_PROTOCOL) ) {
        char stringSerial[50] = {0};
        char IP[50] = {0};
        char resp[RESP_MAX_LEN+1] = {0};
        int i = 0;
        char fl_ip = 0;
               
        Serial.println("Init: OK!");
        Serial.println("Conectando na rede GSM...");

        gsm.Disconnect();
        
        if (fl_ip = gsm.GetIP(IP)){
            Serial.println(IP);
        }
        else{
            Serial.println("Obtendo endereço IP...");  
            if ( gsm.Connect(resp, RESP_MAX_LEN) ) {
                Serial.println("Endereço IP:");
                Serial.println(resp);
                fl_ip = true;
            }
        }

        gsm.SocketDisconnect();
        delay(10);
        
        if ( gsm.SocketConnect(SERVER, PORT) && fl_ip) {
            Serial.println("Conectado ao servidor de destino!");
            i = 0;
            while(1)
            {
                while(Serial.available()) {  
                    if (Serial.available() >0) {
                        stringSerial[i++] = Serial.read();
                    }
                    stringSerial[i] = '\0';
                }
                if(stringSerial[0] != '\0'){
                    gsm.SocketWrite(stringSerial);
                    stringSerial[0] = '\0';
                    i = 0;
                }
                delay(1);
                if (gsm.SocketRead(resp, 100, 1000)>=1)
                {
                    Serial.println(resp);
                }
                
            }
        }
        else{
            Serial.println("Erro ao abrir socket!");
        }
    }
    else{
        Serial.println("Erro ao tentar se comunicar com o módulo...");
        
        if(gsm.Reboot()){
            Serial.println("Reiniciando módulo...");
            delay(1000);
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


