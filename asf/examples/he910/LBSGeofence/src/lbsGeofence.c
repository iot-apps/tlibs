/**
 * @file lbsGeofence.c
 *
 * HE910/UE910/UL865/UE866 telit modem		\n
 * EVK2 \n
 * Arduino M0
 *
 * @brief Exemplo Geofence LBS
 * 
 * Esse exemplo mostra como criar um sistema de geofence com LBS:
 * Determina o pertencimento ao interior de um poligono definido por um array de cordenadas, uma localizacao (latitude/longitude), 
 * obtida por triangulacao de ERBs (antenas da rede celular), e realiza uma analise do poligono considerando a grande
 * imprecisao da posicao obtida. 
 *  
 * @version 1.00 
 */
/*  
 * INSTRUÇÕES DE UTILIZAÇÃO
 * 
 * Esse exemplo foi criado para utilização do kit EVK2 da Telit integrado com o arduino M0 PRO.
 * Para correto funcionamento a conexão das placas devem ser de acordo com a seguinte pinangem:
 *  Pino Arduino M0 Pro       Pino EVK2
 *          GND                 GND
 *          10                  C104/RXD
 *          11                  C103/TXD
 *          09                  RESET
 * 
 * O array de coordenadas do poligono devera ser configurado manualmente através da alteração de constantes no código.
 * Para mais informações sobre o exemplo ou sobre a integração do Arduino e do kit da telit veja: https://www.????????????/
 */

#include <asf.h>
#include <string.h>

#include "serial.h"
#include "gsmXE910.h"
#include "lbsGeofence.h"



/******************************************************************************
 *                               DEFINIÇÕES
 ******************************************************************************/

#define VERBOSE         // Exibe os status da conexão.
//#define SHOW_HEAP_FREE  // Exibe a quantidade de memória livre no Heap.

#define LBSGEOFENCE_TASK_PRIORITY      (tskIDLE_PRIORITY + 2)
#define LBSGEOFENCE_TASK_DELAY         (100 / portTICK_PERIOD_MS)

#define DEFAULT_GSM_APN         "gprs.oi.com.br"
#define DEFAULT_GSM_PROTOCOL    "IP"

#define DATA_RECEIVED_MAX_LEN   128
#define CELL_NUMBER             "+83-numero_telefone-"

/******************************************************************************
 *                               PROTÓTIPOS LOCAIS
 ******************************************************************************/
static void lbsGeofenceTask(void *params);
static void LEDS_Init(void);
static char *ftoa(char *a, double f, int precision);

/******************************************************************************
 *                               VARIÁVEIS GLOBAIS
 ******************************************************************************/
static uint8_t       gu8_cid          = 0;
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


void lbsGeofenceInit(void)
{
    gs_tcpInQueue    = xQueueCreate( DATA_RECEIVED_MAX_LEN, sizeof(uint8_t) );
    gs_serialInQueue = xQueueCreate( DATA_RECEIVED_MAX_LEN, sizeof(uint8_t) );
    
    vQueueAddToRegistry(gs_tcpInQueue   , "tcpIn"   );  // Registro necessário para exibição no FreeRTOS Viewer..
    vQueueAddToRegistry(gs_serialInQueue, "serialIn");

    xTaskCreate( lbsGeofenceTask           ,
                 (const char*) "Lbs"  ,
                 configMINIMAL_STACK_SIZE,
                 NULL                    ,
                 LBSGEOFENCE_TASK_PRIORITY ,
                 NULL                     );
}

/******************************************************************************
 *                                APLICAÇÃO
 ******************************************************************************/
static void lbsGeofenceTask(void *params)
{
    LEDS_Init();
    SERIAL_Init();

    char times[30]        = {0};
    char strLatitude[30]  = {0};
    char strLongitude[30] = {0};
    char satelites[30]    = {0};
    char altitude[30]     = {0};
    double latitude       = 0.0;
    double longitude      = 0.0;

#ifdef VERBOSE
    SERIAL_Print("\r\n");
    SERIAL_Print("******************************\r\n");
    SERIAL_Print("* APLICAÇÃO DE EXEMPLO TELIT *\r\n");
    SERIAL_Print("*        LBS GEOFENCE        *\r\n");
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
     * O loop da aplicação principal é responsável
     * pelo gerenciamento das conexões WIFI e TCP.
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

        if ( GSM_getState() == GSM_ST_INITIALIZED)
        {
#ifdef VERBOSE
            SERIAL_Print("Aguardando LBS...");
#endif

            if (GSM_GetFixLBS(&latitude, &longitude))
            {
#ifdef VERBOSE                      
                ftoa(strLatitude, latitude, 6);
                SERIAL_Println(strLatitude);
                
                ftoa(strLongitude, longitude, 6);
                SERIAL_Println(strLongitude);
#endif                   
            }
			else
			{
#ifdef VERBOSE
            SERIAL_Println("Er");
#endif				
			}
        }

        vTaskDelay(LBSGEOFENCE_TASK_DELAY);
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