/**
 * @file Telit_Modem_UE910GL_LBS_Geofence.ino
 *
 * HE910/UE910/UL865/UE866 telit modem		\n
 * EVK2 \n
 * Arduino M0
 *
 * @brief Exemplo Geofence LBS
 * 
 * Esse exemplo mostra como criar um sistema de geofence com LBS:
 * Determina o pertencimento ao interior de um poligono definido por um array de cordenadas, uma localização (latitude/longitude), 
 * obtida por triangulação de ERBs (antenas da rede celular), e realiza uma análise do polígono considerando a grande
 * imprecisão da posição obtida. 
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

#define R                         6371
#define TO_RAD                    (3.1415926536 / 180)
#define RESP_MAX_LEN              300

Uart serialModule (&sercom1, 11, 10, SERCOM_RX_PAD_0, UART_TX_PAD_2);
XE910 gsm(&serialModule);

const uint8_t polySides = 4; // Quantidade de lados do poligono.

//                              A,          B,         C,         D
//float polyX[polySides] = { -7.107206, -7.108143, -7.108930, -7.107962 }; // X
float polyX[polySides] = { -7.106442, -7.105654, -7.106580, -7.107336 };     // X
float polyY[polySides] = { -34.834198, -34.833092, -34.832288, -34.833371 }; // Y

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

// Cálculo da distância por Haversine.
double dist(double th1, double ph1, double th2, double ph2)
{
  double dx, dy, dz;
  ph1 -= ph2;
  ph1 *= TO_RAD, th1 *= TO_RAD, th2 *= TO_RAD;
 
  dz = sin(th1) - sin(th2);
  dx = cos(ph1) * cos(th1) - cos(th2);
  dy = sin(ph1) * cos(th1);
  return asin(sqrt(dx * dx + dy * dy + dz * dz) / 2) * 2 * R;
}

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

     Serial.println("Testando aplicação Fix GSM...");
     
     delay(1000);
}

void loop() {
    
     if (gsm.ConfigureGSM()){

        char StrDistancia[20] = {0};
        char  StrLatitude[20] = {0};
        char StrLongitude[20] = {0};

        double  latitude = 0.0;                      
        double longitude = 0.0;

        Serial.println("Configure GSM: OK.");
        while(1){
            if(gsm.GetFixLBS(&latitude, &longitude))
            {
                sprintf(StrLatitude, "%s", String(latitude,6).c_str());
                sprintf(StrLongitude, "%s", String(longitude,6).c_str());
                Serial.print("Latitude e longitude: ");
                Serial.print(StrLatitude);
                Serial.print(",");
                Serial.println(StrLongitude);
               
                if (pointInPolygon(latitude, longitude)){
                    Serial.println("Fix pertence ao polígono!");

                }else{
                    Serial.println("Fix não pertence ao polígono!");
                    sprintf(StrDistancia, "%s", String(1000.0*dist(polyX[0], polyY[0], latitude, longitude),3).c_str());
                    Serial.print("Distância do ponto A em metros: ");
                    Serial.println(StrDistancia);

                    sprintf(StrDistancia, "%s", String(1000.0*dist(polyX[1], polyY[1], latitude, longitude),3).c_str());
                    Serial.print("Distância do ponto B em metros: ");
                    Serial.println(StrDistancia);

                    sprintf(StrDistancia, "%s", String(1000.0*dist(polyX[2], polyY[2], latitude, longitude),3).c_str());
                    Serial.print("Distância do ponto C em metros: ");
                    Serial.println(StrDistancia);

                    sprintf(StrDistancia, "%s", String(1000.0*dist(polyX[3], polyY[3], latitude, longitude),3).c_str());
                    Serial.print("Distância do ponto D em metros: ");
                    Serial.println(StrDistancia);


                }
            }
            delay(2000);
            
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
