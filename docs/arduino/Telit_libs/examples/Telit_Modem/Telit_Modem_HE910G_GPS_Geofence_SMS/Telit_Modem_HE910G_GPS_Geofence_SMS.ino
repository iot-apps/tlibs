/**
 * @file Telit_Modem_HE910G_GPS_Geofence_SMS.ino
 *
 * HE910/UE910/UL865/UE866 telit modem		\n
 * EVK2 \n
 * Arduino M0
 *
 * @brief Exemplo Geofence GPS e SMS
 * 
 * Esse exemplo mostra como criar um sistema de geofence com GPS associado a comunicação SMS:
 * Através da solicitação por sms "get fix" o modulo envia uma resposta por sms com os dados resultantes do geofence.
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
 * O array de coordenadas do poligono devera ser configurado manualmente através da alteração de constantes no código.
 * Para mais informações sobre o exemplo ou sobre a integração do Arduino e do kit da telit veja: https://www.????????????/
 */
#define SERIAL_BUFFER_SIZE 200

#include <XE910.h>

#include <Arduino.h>
#include "wiring_private.h"


#define RESP_MAX_LEN              300
#define RESET_PIN                 9
#define BUZZER_PIN                3
#define CELL_NUMBER               "+83-número do telefone-"
#define COMMAND_GET_FIX           "get fix"


Uart serialModule (&sercom1, 11, 10, SERCOM_RX_PAD_0, UART_TX_PAD_2);
XE910 gsm(&serialModule);


const uint8_t polySides = 4; // Quantidade de lados do poligono.

//                              A,        B,         C,          D
//float polyX[polySides] = { -8.107206, -8.108143, -8.108930, -8.107962 }; // X
float polyX[polySides] = { -7.107206, -7.108143, -7.108930, -7.107962 }; // X
float polyY[polySides] = { -34.835296, -34.834491, -34.835607, -34.836444 }; // Y

bool pointInPolygon( double x, double y )
{
  int i, j = polySides - 1;
  bool oddNodes = false;

  for ( i = 0; i < polySides; i++ )
  {
    if ( (polyY[i] < y && polyY[j] >= y || polyY[j] < y && polyY[i] >= y) &&  (polyX[i] <= x || polyX[j] <= x) )
    {
      oddNodes ^= ( polyX[i] + (y - polyY[i]) / (polyY[j] - polyY[i]) * (polyX[j] - polyX[i]) < x );
    }

    j = i;
  }

  return oddNodes;
}


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

     pinMode(LED_BUILTIN, OUTPUT);
     pinMode(BUZZER_PIN, OUTPUT);
          
     Serial.println("Testando aplicação GPS Geofence SMS...");
     
        
     delay(1000);
};

void loop()
{
    
    Serial.println("Inicializando...");
    
    //gsm.Reboot();
    //delay(8000);

    if(1){
    //if ( gsm.ConfigureGPS() ) {
       
        char resp[RESP_MAX_LEN+1] = {0};

        char        times[30] = {0};
        char  strLatitude[30] = {0};                      
        char strLongitude[30] = {0};
        char    satelites[30] = {0};
        char     altitude[30] = {0};
        char       StrFix[100] = {0};
        char          tes[20]  = {0};
                
        double latitude = 0.0;                      
        double longitude = 0.0;

        Serial.println("Cofiguração GPS: OK!");
        
        delay(10);

        if (gsm.StartGPS()){
            
            if ( gsm.ConfigureSMS() ) {
                Serial.println("Cofiguração SMS: OK!");
            }
            
            Serial.println("Buscando sinal GPS...");     
            
            delay(500); 

           /*
            while(!gsm.GetFixGPS(times, &latitude, &longitude, satelites, altitude))
            {
                delay(10);  
            }
            */
            delay(500);
            gsm.StopGPS();
            delay(500);
            
            Serial.println("Aguardando comando: get fix .");
            while(1)  
            {
                if(gsm.MessageRead(resp, 100))    
                {    
                    if (!strcmp(resp,COMMAND_GET_FIX))
                    {
                        gsm.StartGPS();
                        delay(2000);

                        Serial.print("Comando recebido: ");
                        Serial.println(resp);
                        
                        if(gsm.GetFixGPS(times, &latitude, &longitude, satelites, altitude))   
                        {    
                            
                            sprintf(strLatitude, "%s", String(latitude,6).c_str());
                            Serial.println(strLatitude);
                            
                            sprintf(strLongitude, "%s", String(longitude,6).c_str());
                            Serial.println(strLongitude);
        
                            if (pointInPolygon(latitude, longitude)){
                                Serial.println("Fix pertence ao polígono!");
                                digitalWrite(LED_BUILTIN, LOW);
                                analogWrite(BUZZER_PIN, 100);
                            }else{
                                Serial.println("Fix não pertence ao polígono!");
                                digitalWrite(LED_BUILTIN, HIGH);
                                analogWrite(BUZZER_PIN, 0);
                            }

                            
                            strcpy(StrFix, "http://maps.google.com/maps?f=q&q=");
                            strcat(StrFix, strLatitude);
                            strcat(StrFix, ",");
                            strcat(StrFix, strLongitude);
                            strcat(StrFix, "&z=16");
                            gsm.MessageWrite(CELL_NUMBER, StrFix);

                        } 
                        else{
                            gsm.MessageWrite(CELL_NUMBER, "Sem sinal GPS!");     
                        }
                        
                    }
                    
                    gsm.StopGPS();
                } //if(gsm.MessageRead(resp, 100))
                delay(100);
            } // while(1)
        }
    }
};

