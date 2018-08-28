/**
 * @file gpsGeofenceSMS.c
 *
 * HE910/UE910/UL865/UE866 telit modem		\n
 * EVK2 \n
 * Arduino M0
 *
 * @brief Exemplo Geofence GPS e SMS
 * 
 * Esse exemplo mostra como criar um sistema de geofence com GPS associado a comunicacao SMS:
 * Atraves da solicitacao por sms "get fix" o modulo envia uma resposta por sms com os dados resultantes do geofence.
 *
 * @version 1.00 
 */

/*
 * INSTRU��ES DE UTILIZA��O
 * 
 * Esse exemplo foi criado para utiliza��o do kit EVK2 da Telit integrado com o arduino M0 PRO.
 * Para correto funcionamento a conex�o das placas devem ser de acordo com a seguinte pinangem:
 *  Pino Arduino M0 Pro       Pino EVK2
 *          GND                 GND
 *          10                  C104/RXD
 *          11                  C103/TXD
 *          09                  RESET
 * 
 * O array de coordenadas do poligono devera ser configurado manualmente atrav�s da altera��o de constantes no c�digo.
 * Para mais informa��es sobre o exemplo ou sobre a integra��o do Arduino e do kit da telit veja: https://www.????????????/
 */

#include <asf.h>
#include <string.h>

#include "serial.h"
#include "gsmXE910.h"
#include "gpsGeofenceSMS.h"



/******************************************************************************
 *                               DEFINI��ES
 ******************************************************************************/

#define VERBOSE         // Exibe os status da conex�o.
//#define SHOW_HEAP_FREE  // Exibe a quantidade de mem�ria livre no Heap.

#define GPSGEOFENCESMS_TASK_PRIORITY      (tskIDLE_PRIORITY + 2)
#define GPSGEOFENCESMS_TASK_DELAY         (100 / portTICK_PERIOD_MS)

#define DEFAULT_GSM_APN         "gprs.oi.com.br"
#define DEFAULT_GSM_PROTOCOL    "IP"

#define DATA_RECEIVED_MAX_LEN   128
#define CELL_NUMBER             "+83-numero_telefone-"

/******************************************************************************
 *                               PROT�TIPOS LOCAIS
 ******************************************************************************/
static void gpsGeofenceSMSTask(void *params);
static void LEDS_Init(void);
static char *ftoa(char *a, double f, int precision);

/******************************************************************************
 *                               VARI�VEIS GLOBAIS
 ******************************************************************************/
static QueueHandle_t gs_tcpInQueue    = NULL;
static QueueHandle_t gs_serialInQueue = NULL;


static void LEDS_Init(void)
{
   struct port_config pin_conf_output = { PORT_PIN_DIR_OUTPUT, PORT_PIN_PULL_NONE, false };
   
    port_pin_set_config(LED_L_PIN , &pin_conf_output);
    port_pin_set_output_level(LED_L_PIN, 0);
    port_pin_set_config(LED_TX_PIN, &pin_conf_output);
    port_pin_set_output_level(LED_TX_PIN, 1);
    port_pin_set_config(LED_RX_PIN, &pin_conf_output);
    port_pin_set_output_level(LED_RX_PIN, 1);
}


void gpsGeofenceSMSInit(void)
{
    gs_tcpInQueue    = xQueueCreate( DATA_RECEIVED_MAX_LEN, sizeof(uint8_t) );
    gs_serialInQueue = xQueueCreate( DATA_RECEIVED_MAX_LEN, sizeof(uint8_t) );
    
    vQueueAddToRegistry(gs_tcpInQueue   , "tcpIn"   );  // Registro necess�rio para exibi��o no FreeRTOS Viewer..
    vQueueAddToRegistry(gs_serialInQueue, "serialIn");

    xTaskCreate( gpsGeofenceSMSTask           ,
                 (const char*) "Gps"  ,
                 configMINIMAL_STACK_SIZE,
                 NULL                    ,
                 GPSGEOFENCESMS_TASK_PRIORITY ,
                 NULL                     );
}

/******************************************************************************
 *                                APLICA��O
 ******************************************************************************/
static void gpsGeofenceSMSTask(void *params)
{
    LEDS_Init();
    SERIAL_Init();

    char times[30]        = {0};
    char strLatitude[15]  = {0};
    char strLongitude[15] = {0};
	char strSMS[30]       = {0};
    char satelites[30]    = {0};
    char altitude[30]     = {0};
    double latitude       = 0.0;
    double longitude      = 0.0;

#ifdef VERBOSE
    SERIAL_Print("\r\n");
    SERIAL_Print("******************************\r\n");
    SERIAL_Print("* APLICA��O DE EXEMPLO TELIT *\r\n");
    SERIAL_Print("*      GPS GEOFENCE SMS      *\r\n");
    SERIAL_Print("******************************\r\n");
    SERIAL_Print("\r\n");
#endif

    /**********************************************
     * Aguarda o GSM ser iniciado com sucesso.
     **********************************************/
    SERIAL_Print("Iniciando...");

    if ( GSM_Init(DEFAULT_GSM_APN, DEFAULT_GSM_PROTOCOL) )
    {
#ifdef VERBOSE
        SERIAL_Println("OK");
#endif
    }
    else
    {
#ifdef VERBOSE
        SERIAL_Println("ER");
#endif
    }

    /**********************************************
     * O loop da aplica��o principal � respons�vel
     * pelo gerenciamento das conex�es WIFI e TCP.
     **********************************************/
    for(;;)
    {
#ifdef SHOW_HEAP_FREE
        {
            char* toPrint = (char*)pvPortMalloc(32);

            sprintf(toPrint, "Heap Free: %d", xPortGetFreeHeapSize());
            SERIAL_Println(toPrint);
            
            vPortFree(toPrint);
        }
#endif

        if ( GSM_getState() < GSM_ST_GPS_CONFIGURED && GSM_getState() != GSM_ST_SMS_CONFIGURED)
        {
#ifdef VERBOSE
            SERIAL_Print("Configurando GPS...");
#endif

            if ( GSM_ConfigureGPS() )
            {
#ifdef VERBOSE
                SERIAL_Println("OK");
#endif
            }
            else
            {
#ifdef VERBOSE
                SERIAL_Println("ER");
#endif
            }                
        }
        else
        {
            if ( GSM_getState() != GSM_ST_SMS_CONFIGURED && ( GSM_getState() != GSM_ST_GPS_STARTED))
            {
#ifdef VERBOSE
                SERIAL_Print("Configurando SMS...");
#endif
                if ( GSM_ConfigureSMS() )
                {
#ifdef VERBOSE
                    SERIAL_Println("OK");
#endif
                }
                else
                {
#ifdef VERBOSE
                    SERIAL_Println("ER");
#endif
                }  
            }
            else
            {

                if ( GSM_getState() != GSM_ST_GPS_STARTED)
                {
#ifdef VERBOSE
                    SERIAL_Print("Starting GPS...");
#endif

                    if ( GSM_StartGPS() )
                    {
#ifdef VERBOSE
                        SERIAL_Println("OK");
#endif
                    }
                    else
                    {
#ifdef VERBOSE
                        SERIAL_Println("ER");
#endif
                    }                
                }
                else
                {
    				/****************************************************************/
    				char     ch     = 0   ;
                    uint8_t  bufferIndex  ;
                    char*    buffer = (char*)pvPortMalloc(DATA_RECEIVED_MAX_LEN+1);
    				
    				memset(buffer, 0, sizeof(DATA_RECEIVED_MAX_LEN+1));
    				
                    for (bufferIndex = 0; (bufferIndex < DATA_RECEIVED_MAX_LEN) && xQueueReceive(gs_tcpInQueue, &ch, 0); bufferIndex++)
                    {
                        buffer[bufferIndex] = ch;
                    }
                   
    				
    				/****************************************************************/

                    if (GSM_GetFixGPS(buffer, times, &latitude, &longitude, satelites, altitude))
                    {
#ifdef VERBOSE			
						if (latitude != 0.0 && longitude != 0.0)
						{
							ftoa(strLatitude, latitude, 6);
							ftoa(strLongitude, longitude, 6);
							sprintf(strSMS, "%s, %s", strLatitude, strLongitude);
							SERIAL_Println(strSMS);
							GSM_MessageWrite(CELL_NUMBER, strSMS);
							
						}
#endif   
                    }
    				
    				vPortFree(buffer);
                }

            }
        }

        vTaskDelay(GPSGEOFENCESMS_TASK_DELAY);
    }

    vQueueDelete(gs_serialInQueue);
    vQueueDelete(gs_tcpInQueue);
    vTaskDelete(NULL);
}


void SERIAL_ReadCallback(uint8_t data)
{
    //xQueueSendFromISR(gs_serialInQueue, (uint8_t*)&data, NULL);
}

void GSM_TcpReadCallback(uint8_t data)
{
    xQueueSendFromISR(gs_tcpInQueue, (uint8_t*)&data, NULL);
}

static char *ftoa(char *a, double f, int precision)
{
	long p[] = {0,10,100,1000,10000,100000,1000000,10000000,100000000};
	
	char *ret = a;
	long number = (long)f;
	itoa(number, a, 10);
	while (*a != '\0') a++;
	*a++ = '.';
	long decimal = abs((long)((f - number) * p[precision]));
	itoa(decimal, a, 10);
	return ret;
}