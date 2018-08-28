/**
 * @file Telit_Wifi_GS2101M_Logger.ino
 *
 * Telit / GS2101M wifi / EVB3 / Arduino M0
 *
 * @brief Exemplo Logger Wifi
 * 
 * Este é um exemplo de um sistema de logger de Loop infinito (Arduino) baseado em wifi
 * que repassa o tráfego de uma das portas seriais para um socket aberto com um servidor local 
 * Permite também o fluxo de dados servidor->dispositivo, refletido na porta serial. Um tunel de porta serial por TCP.
 * 
 *
 * @version 1.00 
 */
 
/* 
 * INSTRUÇÕES DE UTILIZAÇÃO
 * 
 * Esse exemplo foi criado para utilização da placa EVB3 da Telit integrado com o arduino M0 PRO.
 * Para correto funcionamento a conexão das placas devem ser de acordo com a seguinte pinagem:
 *
 * Pino Arduino M0 Pro	Pino EVB3
 *		GND				  GND
 *		3.3V			VIN_3V3
 *		10			GPIO0/UART0_RX_M
 *		11			GPIO1/UART0_TX_M
 *		09			EXT_RTC_RESET_N
 * 
 * As configurações de servidor, porta, rede e senha deverão ser configurados manualmente através da alteração de constantes no código.
 * Para mais informações sobre o exemplo ou sobre a integração do Arduino e do kit da telit veja: https://www.????????????/
 */
#define SERIAL_BUFFER_SIZE 200

#include <GS2101M.h>

#include <Arduino.h>
#include "wiring_private.h"


#define RESP_MAX_LEN  200
#define RESET_PIN       9

#define SSID_WIFI       "yourNetwork" //Nome de sua rede SSID(name)
#define PWD_WIFI        "yourPassword" //Senha de sua rede (WPA ou WEP key)

#define SERVER          "192.168.0.12"	//Ip do server local a conectar
#define PORT            9000	//Porta de comunicação do server

Uart serialModule (&sercom1, 11, 10, SERCOM_RX_PAD_0, UART_TX_PAD_2);
GS2101M wifi(&serialModule);

void SERCOM1_Handler()
{
  serialModule.IrqHandler();
}

void setup()
{
     Serial.begin(9600);
     serialModule.begin(9600);

     pinPeripheral(10, PIO_SERCOM);
     pinPeripheral(11, PIO_SERCOM);

     delay(1000);
     
     Serial.println("Testando modulo WiFi...");
};

void loop()
{
     Serial.print("Desconectando...");

     if ( wifi.Disconnect() ) {
        Serial.println("OK!");
      } else {
        Serial.println("ERROR!");
        Serial.println("Resetando módulo!");
        wifi.Reset(RESET_PIN);
      }

     Serial.print("Inicializando...");

     if ( wifi.Init() ) {

          char stringSerial[50] = {0};
          char resp[RESP_MAX_LEN+1] = {0};
          char respCid;
          int cid;
          char i = 0;
                    
          Serial.println("OK!\r\nConectando na rede Wifi...");

          if ( wifi.Connect(resp, RESP_MAX_LEN, SSID_WIFI, PWD_WIFI) ) {
            Serial.println(resp);

            if ( (cid = wifi.TcpConnect(SERVER,PORT)) <= 0x0f ){
              Serial.println("TCP OK!\r\n");
              Serial.print("ID da conexão: ");
              Serial.println(cid);
              
                 if (wifi.BData(1)){
                    Serial.println("Envio de dados habilitado!");

                    while(1){
                        
                        while(Serial.available()) {  
                          if (Serial.available() >0) {
                            stringSerial[i++] = Serial.read();
                          }
                        }
                        if(stringSerial[0] != '\0')
                        {
                          wifi.TcpWrite(cid, stringSerial);
                          stringSerial[0] = '\0';
                          i = 0;
                        }
                        if (wifi.TcpRead(cid, resp, 200, 1000)>=1)
                        {
                          Serial.println(resp);
                        }
                    }
                 
                 } else {
                  Serial.println("BDATA ERROR!\r\nDesconectando na rede Wifi...");
                 }
                                 
            }
            else{
              Serial.println("ERROR no CID!");
              Serial.println(respCid);
            }
              
            Serial.print("OK!\r\nDesconectando na rede Wifi...");
                   
            if ( wifi.Disconnect() ) {
              Serial.println("OK!");
            } else {
              Serial.println("ERROR!");
              Serial.println("Resetando módulo!");
              wifi.Reset(RESET_PIN);
            }
                       
          } else {
            Serial.println("ERROR");
            Serial.println("Resetando módulo!");
            wifi.Reset(RESET_PIN);
          }
     } else {
          Serial.println("ERROR!");
          Serial.println("Resetando módulo!");
          wifi.Reset(RESET_PIN);
     }

     while(!Serial.available()) { // Aguarda pressionar uma tecla.
      delay(100);
     }
     
     while(Serial.available()) {
      Serial.read();
      delay(100);
     }
      
};
