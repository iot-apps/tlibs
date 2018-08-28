/**
 * @file  bluetoothBms42.c
 * @brief Biblioteca Wifi para módulo Telit BMS42
 *
 * @author    Jonas Ieno <jonas@lieno.com.br>
 * @date      16/07/2018 09:32:00
 * @warning   -
 * @todo      -
 * @bug       -
 * @copyright Telit - IFPB
 */

#include <asf.h>
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <String.h>

#include "at.h"
#include "bluetoothBms42.h"

#define BT_TASK_PRIORITY  (tskIDLE_PRIORITY + 1)
#define BT_TASK_DELAY     (10 / portTICK_PERIOD_MS)

static void btTaskInit(void);
static void btTask(void *params);
static uint8_t charToByteHex(char charIn);


// Enumeração dos comandos AT:
enum
{
    AT_CMD_AT = 0x00       ,
    AT_CMD_ECHO_OFF        ,
    AT_CMD_ECHO_ON         ,
    AT_CMD_BT_REBOOT       ,
    AT_CMD_BT_CFG_MODE     ,
    AT_CMD_BT_CONNECT      ,

    AT_CMD_SIZE           ,
    AT_CMD_UNKNOWN = 0xff
} __attribute__((packed, aligned(1))) AtCommandsIndex_et;


// Índices dos comandos AT.
static const char* gapc_atCommands[] =
{
    "AT"                ,
    "ATE0"              ,
    "ATE1"              ,
    "AT+RESET"          ,
    "AT+LEROLE"         ,
    "CONNECT"
};

static SemaphoreHandle_t gs_btMutex   = NULL; // Mutex para gerenciar a concorrência de acesso às funções.
static TaskHandle_t      gs_appTask   = NULL; // Armazena a task suspensa enquanto aguarda o término da operação.

static BtState_et        ge_btState   = BT_ST_DEINITIALIZED;    // Estado atual da máquina de estados.
static uint8_t           gu8_btMode   = 0;


/********************************************
 * Task de gerenciamento do módulo bt.
 ********************************************/
static void btTask(void *params)
{
    ge_btState = BT_ST_INITIALIZING;

    AT_Init(gapc_atCommands);

    for(;;)
    {
        xSemaphoreTake(gs_btMutex, portMAX_DELAY);

        switch(ge_btState)
        {
            case BT_ST_DEINITIALIZED:
            {
            }
            break;

            case BT_ST_INITIALIZING:
            {
                switch ( AT_RequestSerial(SERIAL_PORT3, AT_CMD_AT, NULL, NULL, 0, AT_TIMEOUT_DEFAULT) )
                {
                    case AT_ST_RCV_OK:
                        ge_btState = BT_ST_ECHO_OFF;
                    break;

                    case AT_ST_TIMEOUT:
                    case AT_ST_ERROR:
                        ge_btState = BT_ST_INITIALIZING;
                    break;

                    default:
                    break;
                }
				ge_btState = BT_ST_INITIALIZED;
            }
            break;

            case BT_ST_ECHO_OFF:
            {
                switch ( AT_RequestSerial(SERIAL_PORT3, AT_CMD_ECHO_OFF, NULL, NULL, 0, AT_TIMEOUT_DEFAULT) )
                {
                    case AT_ST_RCV_OK:
                        ge_btState = BT_ST_SET_MODE;
                    break;

                    case AT_ST_TIMEOUT:
                    case AT_ST_ERROR:
                        ge_btState = BT_ST_INITIALIZING;
                    break;

                    default:
                    break;
                }
            }
            break;

            case BT_ST_SET_MODE:
            {
               char* modeParams[2] = {0};

               sprintf(modeParams, "%d", gu8_btMode);

               switch ( AT_RequestSerial(SERIAL_PORT3, AT_CMD_BT_CFG_MODE, modeParams, NULL, 0, AT_TIMEOUT_CONNECTION) )
                {
                    case AT_ST_RCV_OK:
                    {
                        ge_btState = BT_ST_INITIALIZED;
                    }                        
                    break;

                    case AT_ST_TIMEOUT:
                    case AT_ST_ERROR:
                        ge_btState = BT_ST_INITIALIZING;
                    break;

                    default:
                    break;
                }
            }
            break;

            case BT_ST_INITIALIZED:
            {
                if (gs_appTask != NULL)
                {
                    vTaskResume(gs_appTask);
                }                    
            }
            break;

            case BT_ST_CONNECT:
            {
               char response[128+1] = {0};

               switch ( AT_RequestSerial(SERIAL_PORT3, AT_CMD_BT_CONNECT, NULL, response, 128, AT_TIMEOUT_CONNECTION) )
                {
                    case AT_ST_RCV_OK_PARAM:
                    {
                        if ( NULL != strstr(response, AT_CMD_BT_CONNECT) ) 
                        {
                            ge_btState = BT_ST_CONNECTED;
                        }
                    }                        
                    break;

                    case AT_ST_TIMEOUT:
                    case AT_ST_ERROR:
                        ge_btState = BT_ST_INITIALIZED;
                    break;

                    default:
                    break;
                }
            }
            break;

            case BT_ST_CONNECTED:
            {
                if (gs_appTask != NULL)
                {
                    vTaskResume(gs_appTask);
                }                    
            }
            break;

            case BT_ST_TCP_TRANSMITTING:
            {
                if (gs_appTask != NULL)
                {
                    vTaskResume(gs_appTask);
                }
            }
            break;

            case BT_ST_ERROR:
            default:
            {
                if (gs_appTask != NULL)
                {
                    vTaskResume(gs_appTask);
                }

                ge_btState = BT_ST_INITIALIZING;
            }
        }

        xSemaphoreGive(gs_btMutex);

        vTaskDelay(BT_TASK_DELAY);
    }

    vTaskDelete(NULL);
}


/*****************************************************
 * Inicializa o módulo Wifi e a task de gerenciamento.
 *****************************************************/
bool BT_Init(uint8_t mode)
{
    gs_btMutex  = xSemaphoreCreateMutex();
    gu8_btMode  = mode;

    vQueueAddToRegistry(gs_btMutex, "bt");

    xSemaphoreTake(gs_btMutex, portMAX_DELAY);    // Mutex.

    xTaskCreate( btTask                ,
                 (const char*) "Bluetooth",
                 configMINIMAL_STACK_SIZE ,
                 NULL                     ,
                 BT_TASK_PRIORITY         ,
                 NULL                      );

    // Aguarda inicialização:
    gs_appTask = xTaskGetCurrentTaskHandle();

    xSemaphoreGive(gs_btMutex);

    vTaskSuspend(gs_appTask);
    gs_appTask = NULL;
	
    return ge_btState == BT_ST_INITIALIZED;
}


/*****************************************************
 * Retorna o estado atual do gerenciamento do módulo.
 *****************************************************/
BtState_et BT_GetState(void)
{
    return ge_btState;
}

/*****************************************************
 * Desinicializa módulo.
 *****************************************************/
void BT_Deinit(void)
{
    AT_Deinit();
}

/*****************************************************
 * Retorna o estado da conexão bt (true se conectado
 * ou false se desconectado.
 *****************************************************/
bool BT_IsConnected(void)
{
    return (ge_btState >= BT_ST_CONNECTED) && (ge_btState < BT_ST_ERROR) ;
}


/*****************************************************
 * Retorna o estado da conexão bt (true se conectado
 * ou false se desconectado.
 *****************************************************/
bool BT_Connect(void)
{
    bool result = false;

    if ( ge_btState == BT_ST_INITIALIZED )
    {
        xSemaphoreTake(gs_btMutex, portMAX_DELAY);

        ge_btState = BT_ST_CONNECTING;

        gs_appTask = xTaskGetCurrentTaskHandle();

        xSemaphoreGive(gs_btMutex);

        vTaskSuspend(gs_appTask);
        gs_appTask = NULL;

        result = BT_IsConnected();
    }

    return result;
}


/*****************************************************
 * Desconecta o bluetooth.
 *****************************************************/
void BT_Disconnect(void)
{
    if ( BT_IsConnected() )
    {
        ge_btState = BT_ST_DISCONNECTING;
    }
}


/*****************************************************
 * Envia o buffer de dados e tamanho.
 *****************************************************/
bool BT_Write(uint8_t* data, uint16_t dataLen)
{
    bool result = false;

    xSemaphoreTake(gs_btMutex, portMAX_DELAY);

    if ( (ge_btState == BT_ST_INITIALIZED) )
    {
        // char* bulkHeader = pvPortMalloc(16+1);

        // sprintf(bulkHeader,"\x1bZ%1x%04d", cid, dataLen);

        // AT_Write(SERIAL_PORT2, bulkHeader, strlen(bulkHeader));

        AT_Write(SERIAL_PORT2, data, dataLen);

        //vPortFree(bulkHeader);

        result = true;
    }

    xSemaphoreGive(gs_btMutex);

    return result;  
}


/*****************************************************
 * Trata a recepção de dados.
 *****************************************************/

#ifndef AT_MULTI_SERIAL
void AT_ReadCallback(uint8_t data, SerialPort_et port)
{
    BT_ReadCallback(data, port);
}
#endif
