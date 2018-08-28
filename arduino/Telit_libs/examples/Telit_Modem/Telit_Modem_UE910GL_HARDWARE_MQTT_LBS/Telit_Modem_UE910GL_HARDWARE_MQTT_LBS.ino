/**
 * @file Telit_Modem_UE910GL_HARDWARE_MQTT_LBS.ino
 *
 * HE910/UE910/UL865/UE866 telit modem		\n
 * EVK2 \n
 * Arduino M0
 *
 * @brief Exemplo sistema MQTT com LBS
 * 
 * Este exemplo mostra como enviar por MQTT (nativo) a localição obtida com tecnlogia LBS, para o portal
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

#define GPRS_OI           "gprs.oi.com.br"
#define GPRS_PROTOCOL     "IP"
#define ENDPOINT          "api-dev.devicewise.com:1883"
#define APP_TOKEN         "pLh5olDkMVIIG8Pd"
                           

#define NUM_MAX_KEY_VALUE     30
#define VARIABLE               0
#define VALUE                  1


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
     
     Serial.println("Testando módulo GSM MQTT HARDWARE LBS");
};

void loop(){

  if ( gsm.Init(GPRS_OI,GPRS_PROTOCOL) ) {
        char stringSerial[50] = {0};
        char IP[50] = {0};
        char resp[RESP_MAX_LEN+1] = {0};
        char debb[100] = {0};
        
        char  StrLatitude[20] = {0};
        char StrLongitude[20] = {0};

        double  latitude = 0.0;                      
        double longitude = 0.0;

        
        int i = 0;
        char fl_ip = 0;
               
        Serial.println("Init: OK!");
        Serial.println("Conectando na rede GSM...");
        
        gsm.CloudDisconnect();
        delay(5000);
        
        gsm.Disconnect();
        delay(5000);
        
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
         
         if ( gsm.CloudConnect(ENDPOINT, APP_TOKEN) && fl_ip) {
            
            Serial.println("Conectado ao Telit IoT Portal!");

            char i = 0;
                        
            while(1){
                
                if(gsm.GetFixLBS(&latitude, &longitude))
                {
                    sprintf(StrLatitude, "%s", String(latitude,6).c_str());
                    sprintf(StrLongitude, "%s", String(longitude,6).c_str());
                
                    gsm.Fix2Cloud("latitude", StrLatitude, 1000);
                    delay(500);
                    gsm.Fix2Cloud("longitude", StrLongitude, 1000);
                 }
                  
                 delay(2000); 
            }
        }
    }
};


