/**
 * @file Telit_Bluetooth_Wifi_Logger.ino
 *
 * BlueMode+S42 / GS2101M telit module \n
 * EVB3 \n
 * BlueEva+S
 * Arduino M0
 *
 * @brief Exemplo Logger Bluetooth Wifi
 * 
 * Este é um exemplo de um sistema de logger de bluetooth baseado em wifi:
 * que repassa o tráfego de um dispositivo por porta serial bluetooth para um socket aberto em um servidor local 
 * Permite também o fluxo de dados refletido na porta serial.
 * 
 *
 * @version 1.00 
 */
 
/* 
 * INSTRUÇÕES DE USO
 * 
 * Esse exemplo foi criado para utilização da placa BlueEva+S integrado com a EVB3 da Telit e com o arduino M0 PRO.
 * Para correto funcionamento a conexão das placas devem ser de acordo com a seguinte pinagem:
 *
 *	Pino Arduino M0 | Pro	Pino BlueEva+S	/	Pino Arduino M0 | Pro Pino EVB3		/	Pino BlueEva+S | Pino EVB3
 *	--------------------------------------  /	---------------------------------	/	--------------------------
 *		3			|	X322/EXT-RES		/	3.3V			|  VIN_3V3			/		ext. PWR   |  VIN_3V3
 *		4			|	X324/UART-RXD		/	GND				|  GND				/		GND		   |  GND
 *		5			|	X323/UART-TXD		/	09				|  EXT_RTC_RESET_N	/	
 *											/	10				|  GPIO0/UART0_RX_M	/	
 *											/	11				|  GPIO1/UART0_TX_M	/
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

#include <GS2101M.h>
#include <BMS42.h>
#include <Arduino.h>   // required before wiring_private.h
#include "wiring_private.h" // pinPeripheral() function

#define RESP_MAX_LEN  200

#define RESET_PIN       9
#define RESET_PIN2      3

#define SSID_WIFI       "Lieno"
#define PWD_WIFI        "yourPassword" //Senha de sua rede (WPA ou WEP key)

#define SERVER          "192.168.0.12"	//Ip do server a conectar
#define PORT            9000	//Porta de comunicação do server

Uart serialModule (&sercom1, 11, 10, SERCOM_RX_PAD_0, UART_TX_PAD_2);
Uart serialModule2 (&sercom2, 5, 4, SERCOM_RX_PAD_3, UART_TX_PAD_2);

GS2101M wifi(&serialModule);
BMS42 BT(&serialModule2);

void SERCOM1_Handler()
{
  serialModule.IrqHandler();
}

void SERCOM2_Handler()
{
  serialModule2.IrqHandler();
}

bool Inicia_Wifi(int cid);
bool Inicia_Bluetooth(void);
 
void setup() {
  Serial.begin(9600);
 
  serialModule.begin(9600); //modulo wifi
  serialModule2.begin(115200); //modulo bluetooth
  
  // Assign pins 10 & 11 SERCOM functionality
  pinPeripheral(10, PIO_SERCOM);
  pinPeripheral(11, PIO_SERCOM);
  pinPeripheral( 5, PIO_SERCOM);
  pinPeripheral( 4, PIO_SERCOM);

  delay (1000);

  BT.Reset(RESET_PIN2);
  Serial.println("Pressione uma tecla");
  while(!Serial.available()) { // Aguarda pressionar uma tecla.
    delay(100);
  }
    while(Serial.available()) {
    Serial.read();
    delay(100);
  }
  /*BT.BtMode(mode);  mode: 0-recebe conexões (bluetooth visível)
                            1-apenas realiza (bluetooth invisivel)
                            2-recebe e realiza conexões btl (bluetooth visivel)
  */
  if(BT.BtMode(1)){
    Serial.println("Testando modulo Bluetooth + Wifi...");
  }
  else
    Serial.println("Erro BT");
}
 
void loop() {
  char resp[RESP_MAX_LEN+1] = {0};
  int cid;

  if (Inicia_Wifi(cid)){
   
      //Serial.println("OK!\r\n");
    if ( Inicia_Bluetooth() ) {
                    
      Serial.println("OK!\r\nAguardando Conexão bluetooth...");
      while ( !BT.BtConnect() ) {}
      Serial.println("Bluetooth OK!\r\n");
    if (wifi.BData(1)){
        Serial.println("Conectado ao servidor de destino!");
        Serial.println("Envio de dados habilitado!\n");
        while(1){

          if (wifi.TcpRead(cid, resp, 200, 1000)>=1)
          {
            BT.BtWrite(resp);
            Serial.println("\nWifi Menssage: ");
            Serial.println(resp);
            //resp[0] = '\0';
          }
          
          if (BT.BtRead(resp, 200, 200)>=1)
          {
            if (NULL != strstr(resp, "0x01") )   // Quando finaliza a transmissão é recebido a mensagem "NO CARRIER 0x01"
            {
              break;
            }
            else  {
              wifi.TcpWrite(cid,resp);
              Serial.println("\nBT Menssage: ");
              Serial.println(resp);
              //resp[0] = '\0';
            }
          }
        }
        Serial.println("\r\nConexão Finalizada!");
        
        delay(10);
      }
      else{
        Serial.println("Erro ao abrir socket!");
        Serial.println("Desconect o bluetooth");
      }                 
    }  
  }           
  Serial.print("OK!\r\nDesconectando na rede Wifi...");
                   
  if ( wifi.Disconnect() ) {
    Serial.println("OK!");
  } 
    else {
      Serial.println("ERROR!");
      Serial.println("Resetando módulo!");
      wifi.Reset(RESET_PIN);
    }
           
  if(BT.BtMode(1)){
    Serial.println("Pressione uma tecla para aguardar nova conexão");
  }
  else
    Serial.println("Erro BT");
      
  while(!Serial.available()) { // Aguarda pressionar uma tecla.
    delay(100);
  }
     
  while(Serial.available()) {
    Serial.read();
    delay(100);
  }
   
}


bool Inicia_Wifi(int cid){
  char resp[RESP_MAX_LEN+1] = {0};
  char respCid;

  Serial.print("Inicializando modulo Wifi...");
  if ( !wifi.Disconnect() ) {
        Serial.println("ERROR!");
        Serial.println("Resetando módulo!");
        wifi.Reset(RESET_PIN);
        Serial.print("Inicializando...");
  }
  if ( wifi.Init() ) {    
    Serial.println("OK!\r\nConectando na rede Wifi...");

    if ( wifi.Connect(resp, RESP_MAX_LEN, SSID_WIFI, PWD_WIFI) ) {
      Serial.println(resp);

      if ( (cid = wifi.TcpConnect(SERVER,PORT)) <= 0x0f ){
        Serial.println("TCP OK!\r\n");
        Serial.print("ID da conexão: ");
        Serial.println(cid);

        return 1;
      }
      else{
        Serial.println("ERROR no CID!");
        Serial.println(respCid);
      }
    }
  }
  Serial.println("ERROR!");
  Serial.println("Resetando módulo!");
  wifi.Reset(RESET_PIN);
  return 0;
}

bool Inicia_Bluetooth(void){
  Serial.print("Inicializando modulo Bluetooth...");
  if ( BT.BtMode(2) ) {
    return 1;
  }
  else {
    Serial.println("ERROR!\r\n");
    return 0;
  }
}

