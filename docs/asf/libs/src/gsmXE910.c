/**
 * @file  gsmGs2101m.c
 * @brief Biblioteca GSM para módulo Telit XE910
 *
 * @author    Jonas Ieno <jonas@lieno.com.br>
 * @date      15/03/2018 16:32:33
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
#include "gsmXE910.h"


#define GSM_TASK_PRIORITY  (tskIDLE_PRIORITY + 1)
#define GSM_TASK_DELAY     (10 / portTICK_PERIOD_MS)

#define NUM_MAX_CIDS        16

#define TCP_HEADER_LEN      7
#define TCP_HEADER_ID_LEN   2


static void    gsmTaskInit(void);
static void    gsmTask(void *params);
static uint8_t charToByteHex(char charIn);
static void    parseIpAddress(char *response);


// Enumeração dos comandos AT:
enum
{
    AT_CMD_AT = 0x00         ,
    AT_CMD_ECHO_OFF          ,
    AT_CMD_ECHO_ON           ,
    AT_CMD_GSM_OPERADORA     ,
    AT_CMD_GSM_CONNECT       ,
    AT_CMD_GSM_SK_CONNECT    ,
    AT_CMD_GSM_SK_DISCONN    ,
    AT_CMD_GSM_SEND          ,
    AT_CMD_GSM_RECV          ,
    AT_CMD_GSM_GETIP         ,
    AT_CMD_GSM_SMS_CSMS      ,
    AT_CMD_GSM_SMS_TEXT      ,
    AT_CMD_GSM_SMS_SEND      ,
    AT_CMD_GSM_SMS_ACKNOW    ,
    AT_CMD_GSM_SMS_CNMI      ,
    AT_CMD_GSM_CONFIG_CLOUD  ,
    AT_CMD_GSM_CONNECT_CLOUD ,
    AT_CMD_GSM_SEND2CLOUD    ,
    AT_CMD_GSM_SEND_REPLY    ,
    AT_CMD_GPS_RST           ,
    AT_CMD_GPS_ClR           ,
    AT_CMD_GPS_ACP           ,
    AT_CMD_GPS_GET           ,
    AT_CMD_GPS_START         ,
    AT_CMD_GSM_FIX           ,
    AT_CMD_GSM_DO_A_CALL     ,
    AT_CMD_GSM_CLOSE_CALL    ,
    AT_CMD_GSM_REBOOT        ,

    AT_CMD_SIZE              ,
    AT_CMD_UNKNOWN = 0xff
} __attribute__((packed, aligned(1))) AtCommandsIndex_et;


// Índices dos comandos AT.
static const char* gapc_atCommands[] =
{
    "AT"           ,
    "ATE0"         ,
    "ATE1"         ,
    "AT+CGDCONT"   ,
    "AT#SGACT"     ,
    "AT#SD"        ,
    "AT#SH"        ,
    "AT#SSEND"     ,
    "AT#SRECV"     ,
    "AT+CGPADDR="  ,
    "AT+CSMS"      ,
    "AT+CMGF"      ,
    "AT+CMGS"      ,
    "AT+CNMA"      ,
    "AT+CNMI"      ,
    "AT#DWCFG"     ,
    "AT#DWCONN"    ,
    "AT#DWSEND"    ,
    "AT#DWRCV"     ,
    "AT$GPSRST"    ,
    "AT$GPSNVRAM"  ,
    "AT$GPSACP"    ,
    "AT$GPSNMUN"   ,
    "AT$GPSP"      ,
    "AT#AGPSSND"   ,
    "ATD "         ,
    "ATH "         ,
    "AT#REBOOT"    ,
};

static SemaphoreHandle_t gs_gsmMutex = NULL; // Mutex para gerenciar a concorrência de acesso às funções.
static TaskHandle_t      gs_appTask  = NULL; // Armazena a task suspensa enquanto aguarda o término da operação.

static GsmState_et       ge_gsmState = GSM_ST_DEINITIALIZED;    // Estado atual da máquina de estados.
static GsmState_et       ge_devState = GSM_ST_DEINITIALIZED;    // Estado atual da máquina de estados.

static char* gca_ip                            = NULL;
static char  gca_protocol[GSM_PROTO_MAX_LEN+1] = {0};
static char  gca_apn[GSM_APN_MAX_LEN+1]        = {0};
static char* gca_TcpAddress                    = NULL;
static char* gca_ApiAddress                    = NULL;
static char* gca_ApiToken                      = NULL;
static char* gca_cloudKey                      = NULL;
static char* gca_cloudValue                    = NULL;
static char* gca_phoneNumber                   = NULL;

static uint16_t gu16_TcpPort = 0;

/********************************************
 * Função utilitária para converter um char
 * em hexa ('0'-'f') em um uint8 (0x00-0x0f).
 ********************************************/
static uint8_t charToByteHex(char charIn)
{
    uint8_t result = charIn;

    if (result >= '0' && result <= '9')
    {
        result -= '0';
    }
    else if (result >= 'a' && result <= 'f')
    {
        result = result - 'a'+10;
    }
    else
    {
        result = 0xff;
    }

    return result;
}

static void parseIpAddress(char *response)
{
    uint8_t i;
    char*   ipStart = NULL;

    // Localiza o IP na resposta:
    for (i = 0; i < strlen(response); i++)
    {
        // Se for número:
        if ( (ipStart == NULL) && (response[i] >= '0') && (response[i] <= '9') )
        {
            ipStart = response + i;
        }

        // Se encontrar o final do IP:
        if (response[i] == ':')
        {
            response[i] = '\0';
            strcpy(gca_ip, ipStart);
            break;
        }
    }
}

/********************************************
 * Task de gerenciamento do módulo gsm.
 ********************************************/
static void gsmTask(void *params)
{
    ge_gsmState = GSM_ST_INITIALIZING;

    gca_ip          = (char*)pvPortMalloc(GSM_IP_MAX_LEN+1);
    gca_TcpAddress  = (char*)pvPortMalloc(GSM_TCP_ADDR_MAX_LEN+1);
    gca_ApiAddress  = (char*)pvPortMalloc(GSM_TCP_ADDR_MAX_LEN+1);
    gca_ApiToken    = (char*)pvPortMalloc(GSM_TCP_ADDR_MAX_LEN+1);
    gca_cloudKey    = (char*)pvPortMalloc(GSM_API_KEY_MAX_LEN+1);
    gca_ApiToken    = (char*)pvPortMalloc(GSM_API_VALUE_MAX_LEN+1);
    gca_phoneNumber = (char*)pvPortMalloc(GSM_PHONE_MAX_LEN+1);

    AT_Init(gapc_atCommands);

    for(;;)
    {
        xSemaphoreTake(gs_gsmMutex, portMAX_DELAY);

        switch(ge_gsmState)
        {
            case GSM_ST_DEINITIALIZED:
            {
            }
            break;

            case GSM_ST_INITIALIZING:
            {
                switch ( AT_Request(AT_CMD_AT, NULL, NULL, 0, AT_TIMEOUT_DEFAULT) )
                {
                    case AT_ST_RCV_OK:
                        ge_gsmState = GSM_ST_ECHO_OFF;
                    break;

                    case AT_ST_TIMEOUT:
                    case AT_ST_ERROR:
                        ge_gsmState = GSM_ST_INITIALIZING;
                    break;

                    default:
                    break;
                }
            }
            break;

            case GSM_ST_ECHO_OFF:
            {
                switch ( AT_Request(AT_CMD_ECHO_OFF, NULL, NULL, 0, AT_TIMEOUT_DEFAULT) )
                {
                    case AT_ST_RCV_OK:
                        ge_gsmState = GSM_ST_REGISTER;
                    break;

                    case AT_ST_TIMEOUT:
                    case AT_ST_ERROR:
                        ge_gsmState = GSM_ST_INITIALIZING;
                    break;

                    default:
                    break;
                }
            }
            break;

            case GSM_ST_REGISTER:
            {
                char* regParams = (char*)pvPortMalloc(128+1);

                sprintf(regParams, "1,%s,%s", gca_protocol, gca_apn);

                switch ( AT_Request(AT_CMD_GSM_OPERADORA, regParams, NULL, 0, AT_TIMEOUT_DEFAULT) )
                {
                    case AT_ST_RCV_OK:
                        ge_gsmState = GSM_ST_INITIALIZED;
                    break;

                    case AT_ST_TIMEOUT:
                    case AT_ST_ERROR:
                        ge_gsmState = GSM_ST_INITIALIZING;
                    break;

                    default:
                    break;
                }

                vPortFree(regParams);
            }
            break;

            case GSM_ST_DISCONNECTING:
            {
                switch ( AT_Request(AT_CMD_GSM_CONNECT, "1,0", NULL, 0, AT_TIMEOUT_DEFAULT) )
                {
                    case AT_ST_RCV_OK:
                        ge_gsmState = GSM_ST_INITIALIZED;
                    break;

                    case AT_ST_TIMEOUT:
                    case AT_ST_ERROR:
                        ge_gsmState = GSM_ST_INITIALIZING;
                    break;

                    default:
                    break;
                }
            }
            break;

            case GSM_ST_INITIALIZED:
            {
                if (gs_appTask != NULL)
                {
                    vTaskResume(gs_appTask);
                }
            }
            break;

            case GSM_ST_CONNECT:
            {
                char response[128+1] = {0};

                switch ( AT_Request(AT_CMD_GSM_CONNECT, "1,1", response, 128, AT_TIMEOUT_CONNECTION) )
                {
					case AT_ST_RCV_OK_PARAM:
                    case AT_ST_RCV_OK:
                    {
                        //TODO: A resposta está vindo com um 0x00 no meio,
                        //isto faz o strlen nao funcionar corretamente.
                        //O melhor seria a API de At retornar o tamanho por
                        //referencia.
                        parseIpAddress(&response);
                        ge_gsmState = GSM_ST_CONNECTED;
                    }
                    break;

                    case AT_ST_TIMEOUT:
                    case AT_ST_ERROR:
                        ge_gsmState = GSM_ST_DISCONNECTING;
                    break;

                    default:
                    break;
                }
            }
            break;

            case GSM_ST_CONNECTED:
            {
                if (gs_appTask != NULL)
                {
                    vTaskResume(gs_appTask);
                }                    
            }
            break;

            case GSM_ST_TCP_CONNECTING:
            {
                char params[GSM_TCP_ADDR_MAX_LEN+20+1] = {0};

                sprintf(params, "1,0,%u,%s,0,0,1", gu16_TcpPort, gca_TcpAddress);

                switch ( AT_Request(AT_CMD_GSM_SK_CONNECT, params, NULL, 0, AT_TIMEOUT_CONNECTION) )
                {
                    case AT_ST_RCV_OK:
                    {
                        ge_gsmState = GSM_ST_TCP_CONNECTED;
                        ge_devState |= GSM_ST_TCP_CONNECTED;
                    }
                    break;

                    case AT_ST_TIMEOUT:
                    case AT_ST_ERROR:
                        ge_gsmState = GSM_ST_CONNECTED;
                    break;

                    default:
                    break;
                }

            }
            break;

            case GSM_ST_API_CONNECTING:
            {
                char params[GSM_TCP_ADDR_MAX_LEN+40+1] = {0};

                sprintf(params, "1,%s,0,%s", gca_ApiAddress, gca_ApiToken);

                switch ( AT_Request(AT_CMD_GSM_CONNECT_CLOUD, params, NULL, 0, AT_TIMEOUT_CONNECTION) )
                {
                    case AT_ST_RCV_OK:
                    {
                        ge_gsmState = GSM_ST_API_CONNECTED;
                    }
                    break;

                    case AT_ST_TIMEOUT:
                    case AT_ST_ERROR:
                        ge_gsmState = GSM_ST_CONNECTED;
                    break;

                    default:
                    break;
                }

            }
            break;
            
            case GSM_ST_TCP_CONNECTED:
            case GSM_ST_API_CONNECTED:
            {
                if (gs_appTask != NULL)
                {
                    vTaskResume(gs_appTask);
                }
            }
            break;

            case GSM_ST_TCP_TRANSMITTING:
            {
                if (gs_appTask != NULL)
                {
                    vTaskResume(gs_appTask);
                }
            }
            break;

            case GSM_ST_TCP_DISCONNECTING:
            {
                switch ( AT_Request(AT_CMD_GSM_SK_DISCONN, "1", NULL, 0, AT_TIMEOUT_DEFAULT) )
                {
                    case AT_ST_RCV_OK:
                        ge_gsmState = GSM_ST_CONNECTED;
                    break;

                    case AT_ST_TIMEOUT:
                    case AT_ST_ERROR:
                        ge_gsmState = GSM_ST_INITIALIZING;
                    break;

                    default:
                    break;
                }
            }
            break;

            case GSM_ST_SMS_CFG_CSMS:
            {
                switch ( AT_Request(AT_CMD_GSM_SMS_CSMS, "1", NULL, 0, AT_TIMEOUT_DEFAULT) )
                {
                    case AT_ST_RCV_OK:
                        ge_gsmState = GSM_ST_SMS_CFG_TEXT;
                    break;

                    case AT_ST_TIMEOUT:
                    case AT_ST_ERROR:
                        ge_gsmState = GSM_ST_INITIALIZING;
                    break;

                    default:
                    break;
                }                
            }
            break;

            case GSM_ST_SMS_CFG_TEXT:
            {
                switch ( AT_Request(AT_CMD_GSM_SMS_TEXT, "1", NULL, 0, AT_TIMEOUT_DEFAULT) )
                {
                    case AT_ST_RCV_OK:
                        ge_gsmState = GSM_ST_SMS_CFG_CNMI;
                    break;

                    case AT_ST_TIMEOUT:
                    case AT_ST_ERROR:
                        ge_gsmState = GSM_ST_INITIALIZING;
                    break;

                    default:
                    break;
                }                
            }
            break;

            case GSM_ST_SMS_CFG_CNMI:
            {
                switch ( AT_Request(AT_CMD_GSM_SMS_CNMI, "2,2,0,0,0", NULL, 0, AT_TIMEOUT_DEFAULT) )
                {
                    case AT_ST_RCV_OK:
                        //TODO: Como sair desse estado? SMS_DISCONNECT?
                        ge_gsmState = GSM_ST_SMS_CONFIGURED;
                        ge_devState |= GSM_ST_SMS_CONFIGURED;
                    break;

                    case AT_ST_TIMEOUT:
                    case AT_ST_ERROR:
                        ge_gsmState = GSM_ST_INITIALIZING;
                    break;

                    default:
                    break;
                }                
            }
            break;

            case GSM_ST_GPS_CFG_RST:
            {
                switch ( AT_Request(AT_CMD_GPS_RST, NULL, NULL, 0, AT_TIMEOUT_DEFAULT) )
                {
                    case AT_ST_RCV_OK:
                        ge_gsmState = GSM_ST_GPS_CFG_ClR;
                    break;

                    case AT_ST_TIMEOUT:
                    case AT_ST_ERROR:
                        ge_gsmState = GSM_ST_INITIALIZING;
                    break;

                    default:
                    break;
                }                
            }
            break;

            case GSM_ST_GPS_CFG_ClR:
            {
                switch ( AT_Request(AT_CMD_GPS_ClR, "15,0", NULL, 0, AT_TIMEOUT_DEFAULT) )
                {
                    case AT_ST_RCV_OK:
                        ge_gsmState = GSM_ST_GPS_CFG_ACP;
                    break;

                    case AT_ST_TIMEOUT:
                    case AT_ST_ERROR:
                        ge_gsmState = GSM_ST_INITIALIZING;
                    break;

                    default:
                    break;
                }                
            }
            break;

            case GSM_ST_GPS_CFG_ACP:
            {
                switch ( AT_Request(AT_CMD_GPS_ACP, NULL, NULL, 0, AT_TIMEOUT_DEFAULT) )
                {
                    case AT_ST_RCV_OK:
                        //TODO: Como sair desse estado? GPS_DISCONNECT?
                        ge_gsmState = GSM_ST_GPS_CFG_START;
                    break;

                    case AT_ST_TIMEOUT:
                    case AT_ST_ERROR:
                        ge_gsmState = GSM_ST_INITIALIZING;
                    break;

                    default:
                    break;
                }                
            }
            break;

            case GSM_ST_GPS_CFG_START:
            {
                switch ( AT_Request(AT_CMD_GPS_START, "1", NULL, 0, AT_TIMEOUT_DEFAULT) )
                {
                    case AT_ST_RCV_OK:
                        //TODO: Como sair desse estado? SMS_DISCONNECT?
                        ge_gsmState = GSM_ST_GPS_CONFIGURED;
                        ge_devState |= GSM_ST_GPS_CONFIGURED;
                    break;

                    case AT_ST_TIMEOUT:
                    case AT_ST_ERROR:
                        ge_gsmState = GSM_ST_INITIALIZING;
                    break;

                    default:
                    break;
                }                
            }
            break;

            case GSM_ST_GPS_STARTING:
            {
                switch ( AT_Request(AT_CMD_GPS_GET, "1,1,0,0,0,0,0", NULL, 0, AT_TIMEOUT_DEFAULT) )
                {
                    case AT_ST_RCV_OK:
                        ge_gsmState = GSM_ST_GPS_STARTED;
                    break;

                    case AT_ST_TIMEOUT:
                    case AT_ST_ERROR:
                        ge_gsmState = GSM_ST_INITIALIZING;
                    break;

                    default:
                    break;
                }                
            }
            break;

            case GSM_ST_GPS_STOPPING:
            {
                switch ( AT_Request(AT_CMD_GPS_GET, "0,0,0,0,0,0,0", NULL, 0, AT_TIMEOUT_DEFAULT) )
                {
                    case AT_ST_RCV_OK:
                        ge_gsmState = GSM_ST_INITIALIZED;
                        ge_devState |= GSM_ST_INITIALIZED;
                    break;

                    case AT_ST_TIMEOUT:
                    case AT_ST_ERROR:
                        ge_gsmState = GSM_ST_INITIALIZING;
                    break;

                    default:
                    break;
                }                
            }
            break;

            case GSM_ST_CALL:
            {
                char params[GSM_PHONE_MAX_LEN+1+1] = {0};

                sprintf(params, "%s;", gca_phoneNumber);

                switch ( AT_Request(AT_CMD_GSM_DO_A_CALL, params, NULL, 0, AT_TIMEOUT_CONNECTION) )
                {
                    case AT_ST_RCV_OK:
                    {
                        ge_gsmState = GSM_ST_ON_CALL;
                        ge_devState |= GSM_ST_ON_CALL;
                    }
                    break;

                    case AT_ST_TIMEOUT:
                    case AT_ST_ERROR:
                        ge_gsmState = GSM_ST_CONNECTED;
                    break;

                    default:
                    break;
                }

            }
            break;

            case GSM_ST_CLOSE_CALL:
            {
                switch ( AT_Request(AT_CMD_GSM_CLOSE_CALL, NULL, NULL, 0, AT_TIMEOUT_DEFAULT) )
                {
                    case AT_ST_RCV_OK:
                        ge_gsmState = GSM_ST_CONNECTED;
                        ge_devState |= GSM_ST_CLOSE_CALL;
                    break;

                    case AT_ST_TIMEOUT:
                    case AT_ST_ERROR:
                        ge_gsmState = GSM_ST_INITIALIZING;
                    break;

                    default:
                    break;
                }                
            }
            break;

            case GSM_ST_ON_CALL:
            case GSM_ST_SMS_CONFIGURED:
            case GSM_ST_GPS_CONFIGURED:
			case GSM_ST_GPS_STARTED:
            {
                if (gs_appTask != NULL)
                {
                    vTaskResume(gs_appTask);
                }         
            }
            break;

            case GSM_ST_ERROR:
            default:
            {
                if (gs_appTask != NULL)
                {
                    vTaskResume(gs_appTask);
                }

                ge_gsmState = GSM_ST_INITIALIZING;
            }
        }

        xSemaphoreGive(gs_gsmMutex);

        vTaskDelay(GSM_TASK_DELAY);
    }

    vPortFree(gca_ip);
    vPortFree(gca_TcpAddress);
    vPortFree(gca_ApiAddress);
    vPortFree(gca_ApiToken);
    vPortFree(gca_cloudKey);
    vPortFree(gca_ApiToken);
    vPortFree(gca_phoneNumber);
    
    vTaskDelete(NULL);
}


/*****************************************************
 * Inicializa o módulo Gsm e a task de gerenciamento.
 *****************************************************/
bool GSM_Init(const char* operadora, const char* ip)
{
    gs_gsmMutex  = xSemaphoreCreateMutex();

    vQueueAddToRegistry(gs_gsmMutex, "gsm");

    xSemaphoreTake(gs_gsmMutex, portMAX_DELAY);    // Mutex.

    strncpy(gca_protocol, ip, GSM_PROTO_MAX_LEN );
    strncpy(gca_apn,      operadora, GSM_APN_MAX_LEN);

    xTaskCreate( gsmTask                ,
                 (const char*) "Gsm"    ,
                 configMINIMAL_STACK_SIZE,
                 NULL                    ,
                 GSM_TASK_PRIORITY      ,
                 NULL                     );

    // Aguarda inicialização:
    gs_appTask = xTaskGetCurrentTaskHandle();

    xSemaphoreGive(gs_gsmMutex);

    vTaskSuspend(gs_appTask);
    gs_appTask = NULL;

    return ge_gsmState == GSM_ST_INITIALIZED;
}

bool GSM_Connect(void)
{
    bool result = false;

    if ( ge_gsmState == GSM_ST_INITIALIZED )
    {
        xSemaphoreTake(gs_gsmMutex, portMAX_DELAY);

        ge_gsmState = GSM_ST_CONNECT;

        gs_appTask = xTaskGetCurrentTaskHandle();

        xSemaphoreGive(gs_gsmMutex);

        vTaskSuspend(gs_appTask);
        gs_appTask = NULL;

        result = GSM_IsConnected();
    }

    return result;
}

bool GSM_IsConnected(void)
{
    return (ge_gsmState >= GSM_ST_CONNECTED) && (ge_gsmState < GSM_ST_ERROR) ;
}

bool GSM_Disconnect(void)
{
    if (GSM_IsConnected())
    {
        ge_gsmState = GSM_ST_DISCONNECTING;
    }
}

bool GSM_SocketConnect(char* address, int port)
{
    bool result = false;

    if (GSM_IsConnected())
    {
        xSemaphoreTake(gs_gsmMutex, portMAX_DELAY);

        strncpy(gca_TcpAddress, address, GSM_TCP_ADDR_MAX_LEN );
                gu16_TcpPort  = port;

        ge_gsmState = GSM_ST_TCP_CONNECTING;

        gs_appTask = xTaskGetCurrentTaskHandle();

        xSemaphoreGive(gs_gsmMutex);

        vTaskSuspend(gs_appTask);
        gs_appTask = NULL;

        result = (ge_gsmState == GSM_ST_TCP_CONNECTED);        
    }

    return result;
}
    
bool GSM_SocketWrite(char* buffer,  uint16_t bufferLen)
{
    bool result = false;

    xSemaphoreTake(gs_gsmMutex, portMAX_DELAY);

    if ( (ge_gsmState == GSM_ST_TCP_CONNECTED) )
    {
        char response[128+1] = {0};

        switch ( AT_Request(AT_CMD_GSM_SEND, "1", response, 128, AT_TIMEOUT_CONNECTION) )
        {
			case AT_ST_RCV_OK_PARAM:
            case AT_ST_RCV_OK:
            {
                if ( (strlen(response) > 0) && (NULL != strstr(response, STR_SEND_OK)) )
                {
                    AT_Write(SERIAL_PORT2, buffer, bufferLen );
                    AT_Write(SERIAL_PORT2, "\x1A", 4              );
                }
            }
            break;

            case AT_ST_TIMEOUT:
            case AT_ST_ERROR:
                ge_gsmState = GSM_ST_CONNECTED;
            break;

            default:
            break;
        }        

        result = true;
    }

    xSemaphoreGive(gs_gsmMutex);

    return result;  
}

bool GSM_GetIP(char* address)
{
    bool result = false;
    
    xSemaphoreTake(ge_gsmState, portMAX_DELAY);

    if ( (gca_ip != NULL) && strlen(gca_ip) )
    {
        strncpy(address, gca_ip, GSM_IP_MAX_LEN);
        result = true;
    }

    xSemaphoreGive(ge_gsmState);
}

int GSM_SocketRead(char* buffer, uint16_t maxBufferLen, uint16_t timeout )
{
    //TODO
}
    
bool GSM_SocketDisconnect(void)
{
	bool result; 
	
    if (ge_gsmState == GSM_ST_TCP_CONNECTED)
    {
        xSemaphoreTake(gs_gsmMutex, portMAX_DELAY);

        ge_gsmState = GSM_ST_TCP_DISCONNECTING;

        gs_appTask = xTaskGetCurrentTaskHandle();

        xSemaphoreGive(gs_gsmMutex);

        vTaskSuspend(gs_appTask);
        gs_appTask = NULL;

        result = (ge_gsmState == GSM_ST_CONNECTED);
    }
    
    return result;
}

bool GSM_CloudConnect(char* apiServer, char* appToken)
{
    bool result = false;
    
    if (GSM_IsConnected())
    {
        xSemaphoreTake(gs_gsmMutex, portMAX_DELAY);

        strncpy(gca_ApiAddress, apiServer, GSM_TCP_ADDR_MAX_LEN );
        strncpy(gca_ApiToken,   appToken,  GSM_TCP_ADDR_MAX_LEN );

        ge_gsmState = GSM_ST_API_CONNECTING;

        gs_appTask = xTaskGetCurrentTaskHandle();

        xSemaphoreGive(gs_gsmMutex);

        vTaskSuspend(gs_appTask);
        gs_appTask = NULL;

        result = (ge_gsmState == GSM_ST_API_CONNECTED);
    }

    return result;    
}

bool GSM_Send2Cloud(char* variable, char* value, uint16_t timeout)
{
    bool result = false;

    if (ge_gsmState == GSM_ST_API_CONNECTED)
    {
        strncpy(gca_cloudKey,   variable, GSM_API_KEY_MAX_LEN );
        strncpy(gca_cloudValue, value,  GSM_API_VALUE_MAX_LEN );

        ge_gsmState = GSM_ST_API_SENDING;
        result = true;
    }

    return result;
}

bool GSM_CloudDisconnect(void)
{
    bool result = false;

    if (ge_gsmState == GSM_ST_API_CONNECTED)
    {
        ge_gsmState = GSM_ST_API_DISCONNECTING;
        result = true;
    }

    return result;
}

bool GSM_ConfigureSMS(void)
{
    bool result = false;
    
    if ( ge_gsmState >= GSM_ST_INITIALIZED )
    {
        xSemaphoreTake(gs_gsmMutex, portMAX_DELAY);

        ge_gsmState = GSM_ST_SMS_CFG_CSMS;

        gs_appTask = xTaskGetCurrentTaskHandle();

        xSemaphoreGive(gs_gsmMutex);

        vTaskSuspend(gs_appTask);
        gs_appTask = NULL;

        result = (ge_gsmState == GSM_ST_SMS_CONFIGURED);
    }

    return result;
}

bool GSM_MessageWrite(const char* number, char* buffer)
{
    bool result = false;

    xSemaphoreTake(gs_gsmMutex, portMAX_DELAY);

    if ( (ge_gsmState >= GSM_ST_SMS_CONFIGURED) )
    {
        char response[128+1] = {0};

        switch ( AT_Request(AT_CMD_GSM_SMS_SEND, number, response, 128, AT_TIMEOUT_CONNECTION) )
        {	
			case AT_ST_RCV_OK_PARAM:
            case AT_ST_RCV_OK:
            {
                if ( (strlen(response) > 0) && (NULL != strstr(response, STR_SEND_OK)) )
                {
                    //TODO: Deixar genérico
                    AT_Write(SERIAL_PORT2, buffer, strlen(buffer) );
                    AT_Write(SERIAL_PORT2, "\x1A", 4              );
                }
            }
            break;           

            default:
            break;
        }        

        result = true;
    }

    xSemaphoreGive(gs_gsmMutex);

    return result;  
}

bool GSM_MessageRead(char *receive, char* buffer, uint16_t timeout )
{
    bool result = false;

    char dataReceivedLenStr[64+1] = {0};
    char command[64]  = {0};
    
    if ((strlen(receive) > 0) && (NULL != strstr(receive, STR_MSG_RCV)))
    {       
        char i = 0;
        char j = 0;
        bool head = false;
        
        for (i = 2; i < strlen(receive); i++) {
            
            if ( (receive[i] == 0x0A)  ) {
                i++;
                head = true;
                break;
            }
        }
        for (j = 0; strlen(receive); j++)
        {
            if (receive[i] != 0x0D)
                buffer[j] = receive[i++];
            else
            {   
                buffer[j+1] = '\0';
                break;
            }
        }               
        
        result = true;
        
    }

    return result;
}

bool GSM_ConfigureGPS(void)
{
    bool result = false;
    
    if ( ge_gsmState >= GSM_ST_INITIALIZED )
    {
        xSemaphoreTake(gs_gsmMutex, portMAX_DELAY);

        ge_gsmState = GSM_ST_GPS_CFG_RST;

        gs_appTask = xTaskGetCurrentTaskHandle();

        xSemaphoreGive(gs_gsmMutex);

        vTaskSuspend(gs_appTask);
        gs_appTask = NULL;

        result = (ge_gsmState == GSM_ST_GPS_CONFIGURED);
    }

    return result;
}

bool GSM_StartGPS(void)
{
    bool result = false;
    
    if ( ge_gsmState >= GSM_ST_INITIALIZED )
    {
        xSemaphoreTake(gs_gsmMutex, portMAX_DELAY);

        ge_gsmState = GSM_ST_GPS_STARTING;

        gs_appTask = xTaskGetCurrentTaskHandle();

        xSemaphoreGive(gs_gsmMutex);

        vTaskSuspend(gs_appTask);
        gs_appTask = NULL;

        result = (ge_gsmState == GSM_ST_GPS_STARTED);
    }

    return result;
}

bool GSM_GetFixGPS(const char* receive, char* time, double* latitude, double* longitude, char* satelites, char* altitude)
{
    bool     result             = false;
    char     position[100]      = {0};
    char     resp[256]          = {0};
        
    char     dmmLatitude[30]    = {0};
    char     dmmLongitude[30]   = {0};
    
    char*    tempLate           = NULL;
    char*    tempLonge          = NULL;

    double   lGraus             = 0.0;
    double   lMinutos           = 0.0;
    
    char     latGraus[15]       = {0};
    char     latMinutos[15]     = {0};
    
    char     lonGraus[15]       = {0};
    char     lonMinutos[15]     = {0};
    
    int inHeader                = 0;
    int timeout                 = 10;
        
    if(receive && (NULL != strstr(receive, STR_GPS_RESP)) && strlen(receive) >= 70)
    {
        
        if ((strlen(receive) > 0) && (NULL == strstr(receive, STR_GPS_NOSIGNAL)))
        {
            
            char i = 0; 
            
            for(i = 0; i <= strlen(receive); i++)
            {
                if (receive[i] == 0x0A)
                {
                    inHeader = i++;
                    break;
                }
            }
            
            memcpy(resp, &receive[inHeader],(strlen(receive) - inHeader)); 
            
            if (NULL != strstr(resp, STR_GPS_END_DATA))
            {
            
                char j = 0;
            
                for(j = 20; j <= strlen(resp); j++)
                {
                    if(resp[j] == 0x4D)
                    { 
                        resp[--j] = '\0';
                        break;
                    }
                }
                
                int k = 0;
                uint8_t paramIndex = 0;
                uint8_t respLen    = strlen(resp);
                char*   field      = NULL;
                
                for(k = 0; k < respLen; k++)
                {
                    if (resp[k] == ',')
                    {
                        resp[k] = '\0';
                        switch(paramIndex)
                        {
                            case 0:
                                field = resp+k+1;   // Time
                            break;
                            case 1:         
                                strcpy(time, field);    
                                field = resp+k+1;   // Latitude
                            break;
                            case 2:
                                //S
                                strcpy(dmmLatitude, field);
                                field = resp+k+1;   // S
                            break;
                            case 3:
                                field = resp+k+1;   // Longitude
                            break;
                            case 4:
                                strcpy(dmmLongitude, field);
                                field = resp+k+1;   // W
                                //W
                            break;
                            case 5:
                                field = resp+k+1;   // 1
                                //
                            break;
                            case 6:
                                field = resp+k+1;   // Satelites
                            break;
                            case 7:
                                strcpy(satelites, field);
                                field = resp+k+1;   // 1.00
                            break;
                            case 8:
                                field = resp+k+1;   // Altitude
                            break;

                        }
                        
                        paramIndex++;
                    }
                    strcpy(altitude, field);
                    
                }
                
                tempLate = strtok(dmmLatitude,".");
                strcpy(latMinutos, tempLate+2);
                strcat(latMinutos, ".");
                tempLate = strtok(NULL,".");
                strcat(latMinutos, tempLate);
                
                dmmLatitude[2] = '\0';
                
                lGraus = atof(dmmLatitude);
                lMinutos =  (atof(latMinutos)/60.0);
                *latitude = (-1.0)* (lGraus + lMinutos);
                
                
                tempLonge = strtok(dmmLongitude,".");
                strcpy(lonMinutos, tempLonge+3);
                
                strcat(lonMinutos, ".");
                tempLonge = strtok(NULL,".");
                strcat(lonMinutos, tempLonge);
                
                dmmLongitude[3] = '\0';
                
                lGraus = atof(dmmLongitude);
                lMinutos =  (atof(lonMinutos)/60.0);
                *longitude = (-1.0)* (lGraus + lMinutos);
                
                result = true;
            }
            
        }
        else
        {
            result = false;
        }
    
    }
    
    return result;
}

bool GSM_StopGPS(void)
{
    bool result = false;
    
    if ( ge_gsmState == GSM_ST_GPS_CONFIGURED )
    {
        xSemaphoreTake(gs_gsmMutex, portMAX_DELAY);

        ge_gsmState = GSM_ST_GPS_STOPPING;

        gs_appTask = xTaskGetCurrentTaskHandle();

        xSemaphoreGive(gs_gsmMutex);

        vTaskSuspend(gs_appTask);
        gs_appTask = NULL;

        result = (ge_gsmState == GSM_ST_CONNECTED);
    }

    return result;
}

bool GSM_ConfigureGSM(void)
{
    ge_gsmState = GSM_ST_INITIALIZING;
}

bool GSM_DoACall(char* number)
{
    bool result = false;

    if (ge_gsmState >= GSM_ST_INITIALIZED)
    {
        xSemaphoreTake(gs_gsmMutex, portMAX_DELAY);

        strncpy(gca_phoneNumber, number, GSM_PHONE_MAX_LEN );

        ge_gsmState = GSM_ST_CALL;

        gs_appTask = xTaskGetCurrentTaskHandle();

        xSemaphoreGive(gs_gsmMutex);

        vTaskSuspend(gs_appTask);
        gs_appTask = NULL;

        result = (ge_gsmState == GSM_ST_ON_CALL);
    }

    return result;
}

bool GSM_CloseCall(void)
{
    bool result = false;

    if (ge_gsmState == GSM_ST_ON_CALL)
    {
        xSemaphoreTake(gs_gsmMutex, portMAX_DELAY);

        ge_gsmState = GSM_ST_CALL;

        gs_appTask = xTaskGetCurrentTaskHandle();

        xSemaphoreGive(gs_gsmMutex);

        vTaskSuspend(gs_appTask);
        gs_appTask = NULL;

        result = (ge_gsmState == GSM_ST_CONNECTED);
    }

    return result;
}

bool GSM_GetFixLBS(double* latitude, double* longitude)
{
    bool result = false;

    xSemaphoreTake(gs_gsmMutex, portMAX_DELAY);

    if ( (ge_gsmState >= GSM_ST_INITIALIZED) )
    {
        char response[200+1] = {0};

        switch ( AT_Request(AT_CMD_GSM_FIX, NULL, response, 200, AT_TIMEOUT_CONNECTION) )
        {   
            case AT_ST_RCV_OK_PARAM:
            case AT_ST_RCV_OK:
            {
                if ( (strlen(response) > 0) && (NULL != strstr(response, "#AGPSRING:")) )
                {
                    char    *ptr;
                    uint8_t i = 0;
                    char  StrLatitude[20] = {0};
                    char StrLongitude[20] = {0};

                    for(i = 10 ; i <= strlen(response); i++)
                    {
                        if (response[i] == 0x0D)
                        {
                            response[i] = '\0';
                            break;
                        }
                    }
                    
                    ptr = strtok(response, ",");
                    ptr = strtok(NULL, ",");
                    strcpy(StrLatitude, ptr);
                    ptr = strtok(NULL, ",");
                    strcpy(StrLongitude, ptr);
                    
                    *latitude = atof(StrLatitude);
                    *longitude = atof(StrLongitude);
                    
                    if ((latitude != 0) && (longitude != 0))
                        result = true;
                }
            }
            break;
            case AT_ST_TIMEOUT:
            case AT_ST_ERROR:
                ge_gsmState = GSM_ST_INITIALIZED;
            break;

            default:
            break;
        }        
    }

    xSemaphoreGive(gs_gsmMutex);

    return result;  
}

bool GSM_Fix2Cloud(char* coordenada, char* valor, uint16_t timeout)
{
    //TODO
}

GsmState_et GSM_getState(void)
{
    return ge_gsmState;
}

GsmState_et GSM_getDevState(void)
{
    return ge_devState;
}

/*****************************************************
 * Trata a recepção dos dados TCP.
 *****************************************************/
#ifndef AT_MULTI_SERIAL
void AT_ReadCallback(uint8_t data, SerialPort_et port)
{
    GSM_TcpReadCallback(data);

}
#endif
