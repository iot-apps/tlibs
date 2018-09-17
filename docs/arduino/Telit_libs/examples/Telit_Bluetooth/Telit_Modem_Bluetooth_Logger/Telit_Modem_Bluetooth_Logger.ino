/**
 * @file Telit_Modem_Bluetooth_Logger.ino
 *
 * HE910/UE910/UL865/UE866 telit modem \n
 * EVK2 \n
 * BlueMode+S42 \n
 * BlueEva+S \n
 * Arduino M0
 *
 * @brief Exemplo Logger Bluetooth Modem
 * 
 * Este é um exemplo de um sistema de logger de bluetooth através de modem: o qual coloca o módulo Bluetooth
 * para ouvir beacons e broadcasts / advertisement próximos e os repassa
 * diretamente para um socket aberto no modem.
 *
 * @version 1.00 
 */
 
/* 
 * INSTRUÇÕES DE USO
 * 
 * Esse exemplo foi criado para utilização da placa BlueEva+S integrado com a EVK2 da Telit e com o arduino M0 PRO.
 * Para correto funcionamento a conexão das placas devem ser de acordo com a seguinte pinagem:
 *
 *	Pino Arduino M0 | Pro Pino BlueEva+S	|	 Pino Arduino M0 Pro | Pino EVK2
 *	----------------------------------		|	 -------------------------------
 *		3.3V			ext. PWR			|			9				RESET
 *		GND				GND					|			GND				GND
 *		3				X322/EXT-RES		|			10				C104/RXD
 *		4				X324/UART-RXD		|			11				C103/TXD
 *		5				X323/UART-TXD		|	
 *
 * As configurações de servidor, porta, operadora e protocolo deverão ser configurados manualmente através da alteração de constantes no código.
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

#include <XE910.h>
#include <BMS42.h>
#include <Arduino.h>   // required before wiring_private.h
#include "wiring_private.h" // pinPeripheral() function

#define RESP_MAX_LEN  200
#define RESET_PIN       9
#define RESET_PIN2      3

#define GPRS_OI         "gprs.oi.com.br"	//operadora
#define GPRS_PROTOCOL   "IP"	//protocolo

#define SERVER          "exemplo.net"	//Endereço do server a conectar
#define PORT            9000	//Porta de comunicação do server
 
Uart serialModule (&sercom1, 11, 10, SERCOM_RX_PAD_0, UART_TX_PAD_2);
Uart serialModule2 (&sercom2, 5, 4, SERCOM_RX_PAD_3, UART_TX_PAD_2);

XE910 gsm(&serialModule);
BMS42 BT(&serialModule2);

void SERCOM1_Handler()
{
  serialModule.IrqHandler();
}

void SERCOM2_Handler()
{
  serialModule2.IrqHandler();
}

bool Inicia_Gsm(char fl_ip);
bool Inicia_Bluetooth(void);
 
void setup() {
  Serial.begin(9600);
 
  //Serial2.begin(9600); //modulo wifi
  serialModule.begin(115200); //modulo gsm
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
    Serial.println("Testando modulo Bluetooth + GSM...");
  }
  else
    Serial.println("Erro BT");
}
 
void loop() {
  char stringSerial[50] = {0};
  char IP[50] = {0};
  char resp[RESP_MAX_LEN+1] = {0};
  int i = 0;
  char fl_ip;

  if (Inicia_Gsm(fl_ip)){
   
      //Serial.println("OK!\r\n");
    if ( Inicia_Bluetooth() ) {
                    
      Serial.println("OK!\r\nAguardando Conexão bluetooth...");
      while ( !BT.BtConnect() ) {}
      Serial.println("Bluetooth OK!\r\n");
      Serial.println(fl_ip);
       if ( gsm.SocketConnect(SERVER, PORT)) {
        Serial.println("Conectado ao servidor de destino!");
        Serial.println("Envio de dados habilitado!\n");
        while(1){

          if (gsm.SocketRead(resp, 100, 200)>=1)
          {
            BT.BtWrite(resp);
            Serial.println("\nGSM Menssage: ");
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
              gsm.SocketWrite(resp);
              Serial.println("\nBT Menssage: ");
              Serial.println(resp);
              //resp[0] = '\0';
            }
          }
        }
        Serial.println("\r\nConexão Finalizada!");
        gsm.SocketDisconnect();
        delay(10);
      }
      else{
        Serial.println("Erro ao abrir socket!");
        Serial.println("Desconect o bluetooth");
      }                 
    }  
  }           
  Serial.println("\r\nDesconectando...");
           
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


bool Inicia_Gsm(char fl_ip){
  char IP[50] = {0};
  char resp[RESP_MAX_LEN+1] = {0};
  int i = 0;
  fl_ip = 0;
  Serial.print("Inicializando modulo GSM...");
  if ( gsm.Init(GPRS_OI,GPRS_PROTOCOL) ) {

               
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
    
    return 1;
  }
  else{
    Serial.println("Erro ao tentar se comunicar com o módulo...");
        
    if(gsm.Reboot()){
      Serial.println("Reiniciando módulo...");
      delay(1000);
    }
    return 0;
  }  
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

