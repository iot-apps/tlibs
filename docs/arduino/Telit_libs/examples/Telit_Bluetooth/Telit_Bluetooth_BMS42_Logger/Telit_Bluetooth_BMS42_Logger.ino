/**
 * @file Telit_Bluetooth_BMS42_Logger.ino
 *
 * BlueMode+S42 \n
 * BlueEva+S \n
 * Arduino M0
 *
 * @brief Exemplo Logger Bluetooth 
 * 
 * Este é um exemplo de um sistema de logger baseado em bluetooth:
 * Loop infinito (Arduino) que repassa o tráfego de uma das portas seriais através de uma conexão bluetooth I/O,
 * através dos comandos AT para terminal I/O do modulo.  Permite também o fluxo de dados entre os dispositivo.
 * Um tunel de porta serial por bluetooth.
 * @version 1.00 
 */
 
/* 
 * INSTRUÇÕES DE USO
 * 
 * Esse exemplo foi criado para utilização da placa BlueEva+S integrado com o arduino M0 PRO.
 * Para correto funcionamento a conexão das placas devem ser de acordo com a seguinte pinagem:
 *
 *	Pino Arduino M0 Pro   |   Pino BlueEva+S
 *	------------------------------------------
 *			3.3V				ext. PWR
 *			GND					GND
 *			9					X322/RESET
 *			10					X324/UART-RXD
 *			11					X323/UART-TXD	
 *
 * As configurações de servidor, porta, rede e senha deverão ser configurados manualmente através da alteração de constantes no código.
 * Para mais informações sobre o exemplo ou sobre a integração do Arduino e do kit da telit veja: https://www.????????????/
 *
 * INTERFACE BLUETOOTH
 *  
 *	A telit disponibiliza o aplicativo "Terminal IO Utility" para android e iOS o qual pode ser utilizado 
 * para estabelecer conexões bluetooth de baixa energia entre um smartphone e o BlueEva+S.
 * 	Neste exemplo se faz o uso deste aplicativo para a intefarce de conexão do dispositovo a placa da Telit, onde 
 * através do mesmo é realizado a conexão e troca de dados.
 * O aplicativo "Terminal IO Utility" da Telit pode ser encontrado na AppleStore e na Google Play Store para download.
 */
#define SERIAL_BUFFER_SIZE 200

#include <BMS42.h>

#include <Arduino.h>
#include "wiring_private.h"


#define RESP_MAX_LEN  200
#define RESET_PIN       9

Uart serialModule (&sercom1, 11, 10, SERCOM_RX_PAD_0, UART_TX_PAD_2);
BMS42 BT(&serialModule);

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
     
     
     if(BT.BtMode(1)){
      Serial.println("Testando modulo Bluetooth...");
     }
};

void loop()
{
     Serial.print("Inicializando...");

  /*BT.BtMode(mode);  mode: 0-recebe conexões (bluetooth visível)
                            1-apenas realiza (bluetooth invisivel)
                            2-recebe e realiza conexões btl (bluetooth visivel)
  */
  if ( BT.BtMode(2) ) {
    char stringSerial[50] = {0};
    char resp[RESP_MAX_LEN+1] = {0};
    char i = 0;
                    
    Serial.println("OK!\r\nAguardando Conexão bluetooth...");
   
    while ( !BT.BtConnect() ) {}
    Serial.println("Bluetooth OK!\r\n");
    Serial.println("Envio de dados habilitado!\n");

    while(1){
                       
      while(Serial.available()) {  
        if (Serial.available() >0) {
          stringSerial[i++] = Serial.read();
        }
      }
      if(stringSerial[0] != '\0')
      {
        BT.BtWrite(stringSerial);
        stringSerial[0] = '\0';
        i = 0;
      }
      if (BT.BtRead(resp, 200, 1000)>=1)
      {
        if (NULL != strstr(resp, "0x01") )   // Quando finaliza a transmissão é recebido a mensagem "NO CARRIER 0x01"
        {
          break;
        }
        else  {
          
          Serial.println(resp);
        }
      }
    }
                 
  } 
  else {
    Serial.println("ERROR!\r\nBluetooth invisivél...");
  }
                                 

              
            Serial.println("OK!\r\nDesconectando...");
                       
           
    if(BT.BtMode(1)){
      Serial.println("Pressione uma tecla para aguardar nova conexão");
     }
      
     while(!Serial.available()) { // Aguarda pressionar uma tecla.
      delay(100);
     }
     
     while(Serial.available()) {
      Serial.read();
      delay(100);
     }
      
};
