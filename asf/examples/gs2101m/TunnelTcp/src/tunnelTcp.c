/**
 * @file tunnelTcp.c
 *
 * Telit / GS2101M wifi / EVB3 / Arduino M0
 *
 * @brief Exemplo Logger Wifi
 * 
 * 	Este e um exemplo de um sistema de logger de Tasks continuas (ASF/FreeRTOS) baseado em wifi
 * que repassa o trafego de uma das portas seriais para um socket aberto com um servidor local 
 * Permite tambem o fluxo de dados servidor->dispositivo, refletido na porta serial. Um tunel de porta serial por TCP.
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
 
#include <asf.h>
#include <string.h>

#include "serial.h"
#include "wifiGs2101m.h"
#include "tunnelTcp.h"


/******************************************************************************
 *                               DEFINIÇÕES
 ******************************************************************************/

#define VERBOSE         // Exibe os status da conexão.
//#define SHOW_HEAP_FREE  // Exibe a quantidade de memória livre no Heap.

#define TUNNELTCP_TASK_PRIORITY      (tskIDLE_PRIORITY + 2)
#define TUNNELTCP_TASK_DELAY         (100 / portTICK_PERIOD_MS)

#define WIFI_SSID               "Lieno"
#define WIFI_PASSWORD           "belohorizonte"

#define TCP_DEFAULT_ADDRESS     "192.168.0.22"
#define TCP_DEFAULT_PORT        9000

#define DATA_RECEIVED_MAX_LEN   64


/******************************************************************************
 *                               PROTÓTIPOS LOCAIS
 ******************************************************************************/
static void tunnelTcpTask(void *params);
static void LEDS_Init(void);


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


void tunnelTcpInit(void)
{
    gs_tcpInQueue    = xQueueCreate( DATA_RECEIVED_MAX_LEN, sizeof(uint8_t) );
    gs_serialInQueue = xQueueCreate( DATA_RECEIVED_MAX_LEN, sizeof(uint8_t) );
    
    vQueueAddToRegistry(gs_tcpInQueue   , "tcpIn"   );  // Registro necessário para exibição no FreeRTOS Viewer..
    vQueueAddToRegistry(gs_serialInQueue, "serialIn");

    xTaskCreate( tunnelTcpTask           ,
                 (const char*) "Tunnel"  ,
                 configMINIMAL_STACK_SIZE,
                 NULL                    ,
                 TUNNELTCP_TASK_PRIORITY ,
                 NULL                     );
}

/******************************************************************************
 *                                APLICAÇÃO
 ******************************************************************************/
static void tunnelTcpTask(void *params)
{
    LEDS_Init();
    SERIAL_Init();

#ifdef VERBOSE
    SERIAL_Print("\r\n");
    SERIAL_Print("******************************\r\n");
    SERIAL_Print("* APLICAÇÃO DE EXEMPLO TELIT *\r\n");
    SERIAL_Print("*          TUNEL TCP         *\r\n");
    SERIAL_Print("******************************\r\n");
    SERIAL_Print("\r\n");
#endif

    /**********************************************
     * Aguarda o Wifi ser iniciado com sucesso.
     **********************************************/
    SERIAL_Print("Iniciando...");
    if ( WIFI_Init() )
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

        if ( !WIFI_IsConnected() )
        {
#ifdef VERBOSE
            SERIAL_Print("Conectando...");
#endif

            if ( WIFI_Connect(WIFI_SSID, WIFI_PASSWORD) )
            {
                char* ip = (char*)pvPortMalloc(16);

                if ( WIFI_GetIp(ip) )
                {
#ifdef VERBOSE
                    SERIAL_Println("OK");

                    SERIAL_Print("  Meu IP: ");
                    SERIAL_Println(ip);
#endif
                }

                vPortFree(ip);
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
            if ( WIFI_TcpGetCids() == 0 )
            {
#ifdef VERBOSE
                SERIAL_Print("Conectando TCP...");
#endif
                
                if ( (gu8_cid = WIFI_TcpConnect(TCP_DEFAULT_ADDRESS, TCP_DEFAULT_PORT)) <= 0x0f )
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
            else // Tunnel:
            {
                char     ch     = 0   ;
                uint8_t  bufferIndex  ;
                uint8_t* buffer = (uint8_t*)pvPortMalloc(DATA_RECEIVED_MAX_LEN+1);

                // Se houver dados na fila da serial, envia para o tcp
                for (bufferIndex = 0; (bufferIndex < DATA_RECEIVED_MAX_LEN) && xQueueReceive(gs_serialInQueue, &ch, 0); bufferIndex++)
                {
                    buffer[bufferIndex] = ch;
                }

                if (bufferIndex > 0)
                {
                    WIFI_TcpWrite(gu8_cid, buffer, bufferIndex);
                }

                // Se houver dados na fila do tcp, envia para a serial
                for (bufferIndex = 0; (bufferIndex < DATA_RECEIVED_MAX_LEN) && xQueueReceive(gs_tcpInQueue, &ch, 0); bufferIndex++)
                {
                    buffer[bufferIndex] = ch;
                }

                if (bufferIndex > 0)
                {
                    SERIAL_Write(buffer, bufferIndex);
                }

                vPortFree(buffer);
            }
        }

        vTaskDelay(TUNNELTCP_TASK_DELAY);
    }

    vQueueDelete(gs_serialInQueue);
    vQueueDelete(gs_tcpInQueue);
    vTaskDelete(NULL);
}


void SERIAL_ReadCallback(uint8_t data)
{
    xQueueSendFromISR(gs_serialInQueue, (uint8_t*)&data, NULL);
}


void WIFI_TcpReadCallback(uint8_t data, SerialPort_et port)
{
    xQueueSendFromISR(gs_tcpInQueue, (uint8_t*)&data, NULL);
}
