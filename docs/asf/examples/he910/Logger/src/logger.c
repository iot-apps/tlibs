/**
 * @file logger.c
 *
 * HE910/UE910/UL865/UE866 telit modem		\n
 * EVK2 \n
 * Arduino M0
 *
 * @brief Exemplo logger GSM
 * 
 * 	Este e um exemplo de um sistema de logger baseado em modem de Tasks contínuas (ASF/FreeRTOS) 
 * que repassa o trafego de uma das portas seriais para um socket aberto com um servidor remoto e 
 * bufferiza os bytes que sao recebidos enquanto a conexao nao estava ativa. 
 * O sistema verifica que a conexao caiu (contexto com a APN ou o socket) e tenta continuamente uma reconexao. 
 * Permite também o fluxo de dados servidor->dispositivo, refletido na porta serial. Um tunel de porta serial por TCP.
 * 
 * @version 1.00 
 */
/*
 * INSTRUÇÕES DE UTILIZAÇÃO
 * 
 * Esse exemplo foi criado para utilização do kit EVK2 da Telit integrado com o arduino M0 PRO.
 * Para correto funcionamento a conexão das placas devem ser de acordo com a seguinte pinangem:
 *	Pino Arduino M0 Pro		  Pino EVK2
 *			GND					GND
 *			10					C104/RXD
 *			11					C103/TXD
 *			09					RESET
 * 
 * As configurações de servidor, baudrate e afins deverão ser realizadas manualmente, através das constantes no código.
 * Para mais informações sobre o exemplo ou sobre a integração do Arduino e do kit da telit veja: https://www.????????????/
 */

#include <asf.h>
#include <string.h>

#include "serial.h"
#include "gsmXE910.h"
#include "logger.h"


/******************************************************************************
 *                               DEFINIÇÕES
 ******************************************************************************/

#define VERBOSE         // Exibe os status da conexão.
//#define SHOW_HEAP_FREE  // Exibe a quantidade de memória livre no Heap.

#define LOGGER_TASK_PRIORITY      (tskIDLE_PRIORITY + 2)
#define LOGGER_TASK_DELAY         (100 / portTICK_PERIOD_MS)

#define DEFAULT_GSM_APN         "gprs.oi.com.br"
#define DEFAULT_GSM_PROTOCOL    "IP"

#define TCP_DEFAULT_ADDRESS     "google.com.br"
#define TCP_DEFAULT_PORT        80

#define DATA_RECEIVED_MAX_LEN   64


/******************************************************************************
 *                               PROTÓTIPOS LOCAIS
 ******************************************************************************/
static void loggerTask(void *params);
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


void loggerInit(void)
{
    gs_tcpInQueue    = xQueueCreate( DATA_RECEIVED_MAX_LEN, sizeof(uint8_t) );
    gs_serialInQueue = xQueueCreate( DATA_RECEIVED_MAX_LEN, sizeof(uint8_t) );
    
    vQueueAddToRegistry(gs_tcpInQueue   , "tcpIn"   );  // Registro necessário para exibição no FreeRTOS Viewer..
    vQueueAddToRegistry(gs_serialInQueue, "serialIn");

    xTaskCreate( loggerTask           ,
                 (const char*) "Tunnel"  ,
                 configMINIMAL_STACK_SIZE,
                 NULL                    ,
                 LOGGER_TASK_PRIORITY ,
                 NULL                     );
}

/******************************************************************************
 *                                APLICAÇÃO
 ******************************************************************************/
static void loggerTask(void *params)
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

        if ( !GSM_IsConnected() )
        {
#ifdef VERBOSE
            SERIAL_Print("Conectando...");
#endif

            if ( GSM_Connect() )
            {
#ifdef VERBOSE
                    SERIAL_Println("OK");
#endif                
/*                char* ip = (char*)pvPortMalloc(16);

                if ( GSM_GetIP(ip) )
                {
#ifdef VERBOSE
                    SERIAL_Println("OK");

                    SERIAL_Print("  Meu IP: ");
                    SERIAL_Println(ip);
#endif
                }

                vPortFree(ip);*/
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
            if ( GSM_getState() != GSM_ST_TCP_CONNECTED)
            {
#ifdef VERBOSE
                SERIAL_Print("Conectando TCP...");
#endif
                
                if (GSM_SocketConnect(TCP_DEFAULT_ADDRESS, TCP_DEFAULT_PORT))
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
                    GSM_SocketWrite(buffer, bufferIndex);
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
            }}
        }

        vTaskDelay(LOGGER_TASK_DELAY);
    }

    vQueueDelete(gs_serialInQueue);
    vQueueDelete(gs_tcpInQueue);
    vTaskDelete(NULL);
}


void SERIAL_ReadCallback(uint8_t data)
{
    xQueueSendFromISR(gs_serialInQueue, (uint8_t*)&data, NULL);
}


void GSM_TcpReadCallback(uint8_t data)
{
    xQueueSendFromISR(gs_tcpInQueue, (uint8_t*)&data, NULL);
}
