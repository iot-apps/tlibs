/**
 * @file logger_bt.c
 *
 * Telit BlueMod+S42		\n
 * EVK2 \n
 * Arduino M0
 *
 * @brief Exemplo logger Bluetooth
 * 
 * Este e um exemplo de um sistema de logger baseado em uma Task (ASF) 
 * que repassa o trafego de uma das portas seriais para um socket aberto com um servidor remoto e 
 * bufferiza os bytes que sao recebidos enquanto a conexao nao estava ativa. 
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

#include "at.h"
#include "serial.h"
#include "bluetoothBms42.h"
#include "logger_bt.h"


/******************************************************************************
 *                               DEFINIÇÕES
 ******************************************************************************/

#define VERBOSE         // Exibe os status da conexão.
//#define SHOW_HEAP_FREE  // Exibe a quantidade de memória livre no Heap.

#define LOGGER_TASK_PRIORITY      (tskIDLE_PRIORITY + 2)
#define LOGGER_TASK_DELAY         (100 / portTICK_PERIOD_MS)

#define BT_MODE                    2
#define DATA_RECEIVED_MAX_LEN      64


/******************************************************************************
 *                               PROTÓTIPOS LOCAIS
 ******************************************************************************/
static void loggerTask(void *params);
static void LEDS_Init(void);


/******************************************************************************
 *                               VARIÁVEIS GLOBAIS
 ******************************************************************************/

static QueueHandle_t gs_btInQueue    = NULL;
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
    gs_btInQueue    = xQueueCreate( DATA_RECEIVED_MAX_LEN, sizeof(uint8_t) );
    gs_serialInQueue = xQueueCreate( DATA_RECEIVED_MAX_LEN, sizeof(uint8_t) );
    
    vQueueAddToRegistry(gs_btInQueue   ,  "btIn");  // Registro necessário para exibição no FreeRTOS Viewer..
    vQueueAddToRegistry(gs_serialInQueue, "serialIn");

    xTaskCreate( loggerTask              ,
                 (const char*) "Logger"  ,
                 configMINIMAL_STACK_SIZE,
                 NULL                    ,
                 LOGGER_TASK_PRIORITY    ,
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
    SERIAL_Print("*          LOGGER            *\r\n");
    SERIAL_Print("******************************\r\n");
    SERIAL_Print("\r\n");
#endif

    /**********************************************
     * Aguarda o Bluetooth ser iniciado com sucesso.
     **********************************************/
    SERIAL_Print("Iniciando...");

    if ( BT_Init(BT_MODE) )
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
     * pelo gerenciamento das conexões.
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

        if ( false/*!BT_IsConnected()*/ )
        {
#ifdef VERBOSE
            SERIAL_Print("Conectando...");
#endif

            if ( BT_Connect() )
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
            char     ch     = 0;
            uint8_t  bufferIndex = 0;
            uint8_t* buffer = (uint8_t*)pvPortMalloc(DATA_RECEIVED_MAX_LEN+1);

            // Se houver dados na fila da serial, envia para o tcp
            for (bufferIndex = 0; (bufferIndex < DATA_RECEIVED_MAX_LEN) && xQueueReceive(gs_serialInQueue, &ch, 0); bufferIndex++)
            {
                buffer[bufferIndex] = ch;
            }

            if (bufferIndex > 0)
            {
                BT_Write(buffer, bufferIndex);
            }
			
			memset(buffer, 0, DATA_RECEIVED_MAX_LEN+1);

            // Se houver dados na fila do tcp, envia para a serial
            for (bufferIndex = 0; (bufferIndex < DATA_RECEIVED_MAX_LEN) && xQueueReceive(gs_btInQueue, &ch, 0); bufferIndex++)
            {
                buffer[bufferIndex] = ch;
            }

            if (bufferIndex > 0)
            {
                SERIAL_Write(buffer, bufferIndex);
            }

            vPortFree(buffer);
        }

        vTaskDelay(LOGGER_TASK_DELAY);
    }

    vQueueDelete(gs_serialInQueue);
    vQueueDelete(gs_btInQueue);
    vTaskDelete(NULL);
}


void SERIAL_ReadCallback(uint8_t data)
{
    xQueueSendFromISR(gs_serialInQueue, (uint8_t*)&data, NULL);
}


void BT_ReadCallback(uint8_t data, SerialPort_et port)
{
    xQueueSendFromISR(gs_btInQueue, (uint8_t*)&data, NULL);
}
