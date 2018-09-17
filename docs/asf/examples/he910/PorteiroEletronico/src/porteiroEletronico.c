/**
 * @file porteiroEletronico.c
 *
 * HE910/UE910/UL865/UE866 telit modem		\n
 * EVK2 \n
 * Arduino M0
 *
 * @brief Exemplo porteiro eletronico
 * 
 * Esse exemplo mostra como criar um sistema que permite ao acionar um botao a realizacao de uma ligacao para um numero predefinido  
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
 * As demais conexões e configurações fica a escolha do usuario.
 * Para mais informações sobre o exemplo ou sobre a integração do Arduino e do kit da telit veja: https://www.????????????/
 */

#include <asf.h>
#include <string.h>

#include "serial.h"
#include "gsmXE910.h"
#include "porteiroEletronico.h"


/******************************************************************************
 *                               DEFINIÇÕES
 ******************************************************************************/

#define VERBOSE         // Exibe os status da conexão.
//#define SHOW_HEAP_FREE  // Exibe a quantidade de memória livre no Heap.

#define PORTEIROELETRONICO_TASK_PRIORITY (tskIDLE_PRIORITY + 2)
#define PORTEIROELETRONICO_TASK_DELAY    (100 / portTICK_PERIOD_MS)

#define DEFAULT_GSM_APN         "gprs.oi.com.br"
#define DEFAULT_GSM_PROTOCOL    "IP"
#define PHONE_NUMBER            "+5583999572818"

#define DATA_RECEIVED_MAX_LEN   64


/******************************************************************************
 *                               PROTÓTIPOS LOCAIS
 ******************************************************************************/
static void porteiroEletronicoTask(void *params);
static void LEDS_Init(void);


/******************************************************************************
 *                               VARIÁVEIS GLOBAIS
 ******************************************************************************/
static uint8_t       gu8_cid          = 0;
static QueueHandle_t gs_tcpInQueue    = NULL;


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


void porteiroEletronicoInit(void)
{
    gs_tcpInQueue    = xQueueCreate( DATA_RECEIVED_MAX_LEN, sizeof(uint8_t) );
    
    vQueueAddToRegistry(gs_tcpInQueue   , "tcpIn"   );  // Registro necessário para exibição no FreeRTOS Viewer..

    xTaskCreate( porteiroEletronicoTask           ,
                 (const char*) "Porteiro"  ,
                 configMINIMAL_STACK_SIZE,
                 NULL                    ,
                 PORTEIROELETRONICO_TASK_PRIORITY ,
                 NULL                     );
}

/******************************************************************************
 *                                APLICAÇÃO
 ******************************************************************************/
static void porteiroEletronicoTask(void *params)
{
    LEDS_Init();
    SERIAL_Init();

#ifdef VERBOSE
    SERIAL_Print("\r\n");
    SERIAL_Print("******************************\r\n");
    SERIAL_Print("* APLICAÇÃO DE EXEMPLO TELIT *\r\n");
    SERIAL_Print("*    PORTEIRO ELETRONICO     *\r\n");
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
            SERIAL_Print("Calling...");
            
            if ( GSM_DoACall(PHONE_NUMBER) )
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

        vTaskDelay(PORTEIROELETRONICO_TASK_DELAY);
    }

    vQueueDelete(gs_tcpInQueue);
    vTaskDelete(NULL);
}


void SERIAL_ReadCallback(uint8_t data)
{

}


void GSM_TcpReadCallback(uint8_t data)
{
    xQueueSendFromISR(gs_tcpInQueue, (uint8_t*)&data, NULL);
}
