/**
 * @file smsCommands.c
 *
 * HE910/UE910/UL865/UE866 telit modem		\n
 * EVK2 \n
 * Arduino M0
 *
 * @brief Exemplo comandos por SMS
 * 
 *	Este exemplo mostra como executar comandos atraves de Comunicacao por SMS: 
 * Tasks ou rotinas que leem continuamente as mensagens SMS recebidas pelo modulo, 
 * lendo seu conteudo e executando comandos conforme o texto enviado pela mensagem e com resposta(feedback) por SMS.
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
 * Para mais informações sobre o exemplo ou sobre a integração do Arduino e do kit da telit veja: https://www.????????????/
 */

#include <asf.h>
#include <string.h>

#include "serial.h"
#include "gsmXE910.h"
#include "smsCommands.h"


/******************************************************************************
 *                               DEFINIÇÕES
 ******************************************************************************/

#define VERBOSE         // Exibe os status da conexão.
//#define SHOW_HEAP_FREE  // Exibe a quantidade de memória livre no Heap.

#define SMSCOMMANDS_TASK_PRIORITY      (tskIDLE_PRIORITY + 2)
#define SMSCOMMANDS_TASK_DELAY         (100 / portTICK_PERIOD_MS)

#define DEFAULT_GSM_APN         "gprs.oi.com.br"
#define DEFAULT_GSM_PROTOCOL    "IP"

#define DATA_RECEIVED_MAX_LEN   64
#define CELL_NUMBER             "+83-numero_telefone-"

/******************************************************************************
 *                               PROTÓTIPOS LOCAIS
 ******************************************************************************/
static void smsCommandsTask(void *params);
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


void smsCommandsInit(void)
{
    gs_tcpInQueue    = xQueueCreate( DATA_RECEIVED_MAX_LEN, sizeof(uint8_t) );
    gs_serialInQueue = xQueueCreate( DATA_RECEIVED_MAX_LEN, sizeof(uint8_t) );
    
    vQueueAddToRegistry(gs_tcpInQueue   , "tcpIn"   );  // Registro necessário para exibição no FreeRTOS Viewer..
    vQueueAddToRegistry(gs_serialInQueue, "serialIn");

    xTaskCreate( smsCommandsTask           ,
                 (const char*) "Sms"  ,
                 configMINIMAL_STACK_SIZE,
                 NULL                    ,
                 SMSCOMMANDS_TASK_PRIORITY ,
                 NULL                     );
}

/******************************************************************************
 *                                APLICAÇÃO
 ******************************************************************************/
static void smsCommandsTask(void *params)
{
    LEDS_Init();
    SERIAL_Init();

#ifdef VERBOSE
    SERIAL_Print("\r\n");
    SERIAL_Print("******************************\r\n");
    SERIAL_Print("* APLICAÇÃO DE EXEMPLO TELIT *\r\n");
    SERIAL_Print("*        SMS COMMANDS        *\r\n");
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

        if ( GSM_getState() != GSM_ST_SMS_CONFIGURED)
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
#ifdef VERBOSE
                SERIAL_Println("SMS Configurado...");
                SERIAL_Println("Enviando mensagem.");
#endif   
         GSM_MessageWrite(CELL_NUMBER, "Teste ASF");

        }

        vTaskDelay(SMSCOMMANDS_TASK_DELAY);
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
