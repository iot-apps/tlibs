/**
 * @file  wifiGs2101m.c
 * @brief Biblioteca Wifi para módulo Telit GS2101M
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
#include "wifiGs2101m.h"


#define WIFI_TASK_PRIORITY  (tskIDLE_PRIORITY + 1)
#define WIFI_TASK_DELAY     (10 / portTICK_PERIOD_MS)

#define NUM_MAX_CIDS        16

#define TCP_HEADER_LEN      7
#define TCP_HEADER_ID_LEN   2


static void wifiTaskInit(void);
static void wifiTask(void *params);
static uint8_t charToByteHex(char charIn);


// Enumeração dos comandos AT:
enum
{
    AT_CMD_AT = 0x00       ,
    AT_CMD_ECHO_OFF        ,
    AT_CMD_ECHO_ON         ,
    AT_CMD_WIFI_DISCONNECT ,
    AT_CMD_WIFI_SET_MODE   ,
    AT_CMD_WIFI_DHCP       ,
    AT_CMD_WIFI_WPA        ,
    AT_CMD_WIFI_CONNECT    ,
    AT_CMD_CID             ,
    AT_CMD_TCP_CONNECT     ,
    AT_CMD_BDATA           ,
    AT_CMD_MQTT_CONNECT    ,
    AT_CMD_MQTT_PUBLISH    ,
    AT_CMD_MQTT_SUBSCRIBE  ,
    AT_CMD_MQTT_DISCONNECT ,
    AT_CMD_MQTT_STORE      ,
    AT_CMD_MQTT_RESTORE    ,
    AT_CMD_MQTT_PING       ,

    AT_CMD_SIZE           ,
    AT_CMD_UNKNOWN = 0xff
} __attribute__((packed, aligned(1))) AtCommandsIndex_et;


// Índices dos comandos AT.
static const char* gapc_atCommands[] =
{
    "AT"                ,
    "ATE0"              ,
    "ATE1"              ,
    "AT+WD"             ,
    "AT+WM"             ,
    "AT+NDHCP"          ,
    "AT+WWPA"           ,
    "AT+WA"             ,
    "AT+CID"            ,
    "AT+NCTCP"          ,
    "AT+BDATA"          ,
    "AT+MQTTCONNECT"    ,
    "AT+MQTTPUBLISH"    ,
    "AT+MQTTSUBSCRIBE"  ,
    "AT+MQTTDISCONNECT" ,
    "AT+MQTTSTORE"      ,
    "AT+MQTTRESTORE"    ,
    "AT+MQTTPING"
};

static SemaphoreHandle_t gs_wifiMutex   = NULL; // Mutex para gerenciar a concorrência de acesso às funções.
static TaskHandle_t      gs_appTask     = NULL; // Armazena a task suspensa enquanto aguarda o término da operação.

static WifiState_et      ge_wifiState   = WIFI_ST_DEINITIALIZED;    // Estado atual da máquina de estados.

static char* gca_ssid = NULL;
static char* gca_pass = NULL;
static char* gca_ip   = NULL;

static char*    gca_addressToConnectTcp = NULL;
static uint16_t gu16_portToConnectTcp   =  0;

static uint16_t gu16_tcpCheckCidCount = 0;
static uint16_t gu16_cidsMask         = 0x0000; // Máscara com os CIDs ativos.
static uint8_t  gu8_currentCid        = 0xff;   // CID em operação.

static char*    gac_tcpHdrStr     = NULL;
static uint8_t  gu8_tcpHdrIndex   = 0;
static uint32_t gu32_tcpDataLen   = 0;
static uint8_t  gu8_tcpDataIndex  = 0;
static bool     gb_tcpHdrReceived = false;

/*
 * MQTT Globals
 */

static char* gca_addr     = NULL;
static uint16_t gu16_port =  0;
static char* gca_clientId = NULL;

static uint16_t gu16_mqttCidsMask  = 0x0000; // Máscara com os CIDs ativos.
static uint8_t  gu8_mqttCurrentCid = 0xff;   // CID em operação.

static QueueHandle_t gq_mqttMsgs   = NULL;
static QueueHandle_t gq_mqttTopics = NULL;

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

/********************************************
 * Task de gerenciamento do módulo wifi.
 ********************************************/
static void wifiTask(void *params)
{
    gca_ssid = (char*)pvPortMalloc(WIFI_SSID_MAX_LEN+1);
    gca_pass = (char*)pvPortMalloc(WIFI_PASS_MAX_LEN+1);
    gca_ip   = (char*)pvPortMalloc(WIFI_IP_MAX_LEN  +1);

    gca_addressToConnectTcp = (char*)pvPortMalloc(WIFI_ADDR_TCP_MAX_LEN+1);

    gac_tcpHdrStr = (char*)pvPortMalloc(TCP_HEADER_LEN+1);
    strcpy(gac_tcpHdrStr, "\x1bZcllll");

    gca_addr     = (char*)pvPortMalloc(MQTT_ADDRESS_MAX_LEN+1);
    gca_clientId = (char*)pvPortMalloc(MQTT_CLIENT_ID_MAX_LEN+1);;

    ge_wifiState   = WIFI_ST_INITIALIZING;
    gu16_cidsMask  = 0x0000;
    gu8_currentCid = 0xff;

    strcpy(gca_ssid, WIFI_DEFAULT_SSID);
    strcpy(gca_pass, WIFI_DEFAULT_PASS);

    AT_Init(gapc_atCommands);

    for(;;)
    {
        xSemaphoreTake(gs_wifiMutex, portMAX_DELAY);

        switch(ge_wifiState)
        {
            case WIFI_ST_DEINITIALIZED:
            {
            }
            break;

            case WIFI_ST_INITIALIZING:
            {
                switch ( AT_Request(AT_CMD_AT, NULL, NULL, 0, AT_TIMEOUT_DEFAULT) )
                {
                    case AT_ST_RCV_OK:
                        ge_wifiState = WIFI_ST_ECHO_OFF;
                    break;

                    case AT_ST_TIMEOUT:
                    case AT_ST_ERROR:
                        ge_wifiState = WIFI_ST_INITIALIZING;
                    break;

                    default:
                    break;
                }
            }
            break;

            case WIFI_ST_ECHO_OFF:
            {
                switch ( AT_Request(AT_CMD_ECHO_OFF, NULL, NULL, 0, AT_TIMEOUT_DEFAULT) )
                {
                    case AT_ST_RCV_OK:
                        ge_wifiState = WIFI_ST_SET_MODE;
                    break;

                    case AT_ST_TIMEOUT:
                    case AT_ST_ERROR:
                        ge_wifiState = WIFI_ST_INITIALIZING;
                    break;

                    default:
                    break;
                }
            }
            break;

            case WIFI_ST_SET_MODE:
            {
                switch ( AT_Request(AT_CMD_WIFI_SET_MODE, WIFI_MODE_STATION, NULL, 0, AT_TIMEOUT_DEFAULT) )
                {
                    case AT_ST_RCV_OK:
                        ge_wifiState = WIFI_ST_DISCONNECTING;
                    break;

                    case AT_ST_TIMEOUT:
                    case AT_ST_ERROR:
                        ge_wifiState = WIFI_ST_INITIALIZING;
                    break;

                    default:
                    break;
                }
            }
            break;

            case WIFI_ST_DISCONNECTING:
            {
                switch ( AT_Request(AT_CMD_WIFI_DISCONNECT, NULL, NULL, 0, AT_TIMEOUT_DEFAULT) )
                {
                    case AT_ST_RCV_OK:
                        gu16_cidsMask  = 0x0000;
                        gu8_currentCid = 0xff;

                        ge_wifiState = WIFI_ST_INITIALIZED;
                    break;

                    case AT_ST_TIMEOUT:
                    case AT_ST_ERROR:
                        ge_wifiState = WIFI_ST_INITIALIZING;
                    break;

                    default:
                    break;
                }
            }
            break;

            case WIFI_ST_INITIALIZED:
            {
                if (gs_appTask != NULL)
                {
                    vTaskResume(gs_appTask);
                }                    
            }
            break;

            case WIFI_ST_CONNECTING:
            case WIFI_ST_DHCP:
            {
                char* dhcpParams = (char*)pvPortMalloc(WIFI_SSID_MAX_LEN+4+1);

                sprintf(dhcpParams, "1,%s,1", gca_ssid);

                switch ( AT_Request(AT_CMD_WIFI_DHCP, dhcpParams, NULL, 0, AT_TIMEOUT_DEFAULT) )
                {
                    case AT_ST_RCV_OK:
                        ge_wifiState = WIFI_ST_WPA;
                    break;

                    case AT_ST_TIMEOUT:
                    case AT_ST_ERROR:
                        ge_wifiState = WIFI_ST_INITIALIZED;
                    break;

                    default:
                    break;
                }

                vPortFree(dhcpParams);
            }
            break;

            case WIFI_ST_WPA:
            {
                switch ( AT_Request(AT_CMD_WIFI_WPA, gca_pass, NULL, 0, AT_TIMEOUT_DEFAULT) )
                {
                    case AT_ST_RCV_OK:
                        ge_wifiState = WIFI_ST_CONNECT;
                    break;

                    case AT_ST_TIMEOUT:
                    case AT_ST_ERROR:
                        ge_wifiState = WIFI_ST_INITIALIZED;
                    break;

                    default:
                    break;
                }
            }
            break;

            case WIFI_ST_CONNECT:
            {
               char response[128+1] = {0};

               switch ( AT_Request(AT_CMD_WIFI_CONNECT, gca_ssid, response, 128, AT_TIMEOUT_CONNECTION) )
                {
                    case AT_ST_RCV_OK_PARAM:
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

                        gu16_cidsMask  = 0x0000;
                        gu8_currentCid = 0xff;

                        ge_wifiState = WIFI_ST_CONNECTED;
                    }                        
                    break;

                    case AT_ST_TIMEOUT:
                    case AT_ST_ERROR:
                        ge_wifiState = WIFI_ST_INITIALIZED;
                    break;

                    default:
                    break;
                }
            }
            break;

            case MQTT_ST_CONNECTED:
            case WIFI_ST_CONNECTED:
            {
                if (gs_appTask != NULL)
                {
                    vTaskResume(gs_appTask);
                }                    
            }
            break;

            case WIFI_ST_TCP_CONNECTING:
            {
                char params[WIFI_ADDR_TCP_MAX_LEN+6+1] = {0};
                char response[64+1] = {0};

                sprintf(params, "%s,%u", gca_addressToConnectTcp, gu16_portToConnectTcp);

                switch ( AT_Request(AT_CMD_TCP_CONNECT, params, response, 64, AT_TIMEOUT_CONNECTION) )
                {
                    case AT_ST_RCV_OK_PARAM:
                    {
                        char cid = 0xff;

                        sscanf(response, "\r\nCONNECT %c\r\n\r\nOK\r\n", &cid);

                        cid = charToByteHex(cid);

                        if (cid <= 0x0f)
                        {
                            gu8_currentCid = cid;
                            gu16_cidsMask |= 0x0001 << gu8_currentCid;
                            ge_wifiState   = WIFI_ST_TCP_BDATA;
                        }
                        else
                        {
                            gu8_currentCid = 0xff;
                            gu16_cidsMask  = 0x0000;
                            ge_wifiState   = WIFI_ST_CONNECTED;
                        }
                    }                        
                    break;

                    case AT_ST_TIMEOUT:
                    case AT_ST_ERROR:
                        ge_wifiState = WIFI_ST_CONNECTED;
                    break;

                    default:
                    break;
                }
            }
            break;
            
            case MQTT_ST_CONNECTING:
            {
                char* connectParams = (char*) pvPortMalloc(256+1);
                char response[128+1] = {0};

                sprintf(connectParams, "%s,%u,%s,,,,,,,,", gca_addr, gu16_port, gca_clientId);

                switch ( AT_Request(AT_CMD_MQTT_CONNECT, connectParams, response, 128, AT_TIMEOUT_CONNECTION) )
                {
                    case AT_ST_RCV_OK_PARAM:
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
                                //TODO: Precisa guardar o IP?
                                break;
                            }
                        }

                        gu16_mqttCidsMask  = 0x0000;
                        gu8_mqttCurrentCid = 0xff;

                        ge_wifiState = MQTT_ST_CONNECTED;
                    }                        
                    break;

                    case AT_ST_TIMEOUT:
                    case AT_ST_ERROR:
                        ge_wifiState = WIFI_ST_CONNECTED;
                    break;

                    default:
                    break;
                }

                vPortFree(connectParams);
            }
            break;

            case WIFI_ST_TCP_BDATA:
            {
                switch ( AT_Request(AT_CMD_BDATA, "1", NULL, 0, AT_TIMEOUT_DEFAULT) )
                {
                    case AT_ST_RCV_OK:
                        ge_wifiState = WIFI_ST_TCP_CONNECTED;
                    break;

                    case AT_ST_TIMEOUT:
                    case AT_ST_ERROR:
                        ge_wifiState = WIFI_ST_CONNECTED;
                    break;

                    default:
                    break;
                }
            }
            break;

            case MQTT_ST_SUBSCRIBING:
            {
                if (uxQueueMessagesWaiting(gq_mqttTopics) > 0)
                {
                    MqttTopic_st mqttTopic = {0};

                    if (xQueueReceive(gq_mqttTopics, &mqttTopic, 0) == pdTRUE)
                    {
                        char* subscribeParams = (char*) pvPortMalloc(256+1);

                        sprintf(subscribeParams, "%u,%s,%u,%lu", mqttTopic.cid, mqttTopic.topic, mqttTopic.qos, mqttTopic.timeout);

                        switch ( AT_Request(AT_CMD_MQTT_SUBSCRIBE, subscribeParams, NULL, 0, AT_TIMEOUT_CONNECTION) )
                        {
                            case AT_ST_RCV_OK_PARAM:
                            {
                                if (uxQueueMessagesWaiting(gq_mqttTopics) == 0)
                                    ge_wifiState = MQTT_ST_CONNECTED;
                            }                        
                            break;

                            case AT_ST_TIMEOUT:
                            case AT_ST_ERROR:
                                ge_wifiState = WIFI_ST_CONNECTED;
                            break;

                            default:
                            break;
                        }

                        vPortFree(subscribeParams);
                    }

                }
                else
                {
                    ge_wifiState = MQTT_ST_CONNECTED;
                }
            }
            break;

            case MQTT_ST_PUBLISHING:
            {
                if (uxQueueMessagesWaiting(gq_mqttMsgs) > 0)
                {
                    MqttMessage_st msg = {0};

                    if (xQueueReceive(gq_mqttMsgs, &msg, 0) == pdTRUE)
                    {
                        char* publishParams = (char*) pvPortMalloc(256+1);

                        sprintf(publishParams, "%u,%s,%u,%u,%u", msg.cid, msg.topic, msg.len, msg.qos, msg.retain);

                        switch ( AT_Request(AT_CMD_MQTT_PUBLISH, publishParams, NULL, 0, AT_TIMEOUT_CONNECTION) )
                        {
                            case AT_ST_RCV_OK_PARAM:
                            {
                                MQTT_Write(msg.cid, msg.data, msg.len);
                                
                                if (uxQueueMessagesWaiting(gq_mqttMsgs) == 0)
                                    ge_wifiState = MQTT_ST_CONNECTED;
                            }                        
                            break;

                            case AT_ST_TIMEOUT:
                            case AT_ST_ERROR:
                                ge_wifiState = WIFI_ST_CONNECTED;
                            break;

                            default:
                            break;
                        }

                        vPortFree(publishParams);

                    }

                }
                else
                {
                    ge_wifiState = MQTT_ST_CONNECTED;
                }
            }
            break;

            case MQTT_ST_DISCONNECTING:
            {
                //TODO
            }
            break;

            case WIFI_ST_TCP_CONNECTED:
            {
                if (gs_appTask != NULL)
                {
                    vTaskResume(gs_appTask);
                }

                // A cada 5s verifica se a conexão TCP continua ativa:
                if (gu16_tcpCheckCidCount++ > 500)
                {
                    gu16_tcpCheckCidCount = 0;
                    
                    ge_wifiState = WIFI_ST_TCP_CHECK_CID;
                }
            }
            break;

            case WIFI_ST_TCP_CHECK_CID:
            {
                char response[200+1] = {0};

                switch ( AT_Request(AT_CMD_CID, "?", response, 200, AT_TIMEOUT_CONNECTION) )
                {
                case AT_ST_RCV_OK_PARAM:
                    {
                        // TODO: Tratar múltiplos CIDs.
                        char cid = charToByteHex(response[70]);

                        if (cid <= 0x0f)
                        {
                            gu8_currentCid = cid;
                            gu16_cidsMask |= 0x0001 << gu8_currentCid;
                            ge_wifiState   = WIFI_ST_TCP_CONNECTED;
                        }
                        else
                        {
                            gu8_currentCid = 0xff;
                            gu16_cidsMask  = 0x0000;
                            ge_wifiState   = WIFI_ST_CONNECTED;
                        }
                   }
                   break;

                   case AT_ST_TIMEOUT:
                   case AT_ST_ERROR:
                        ge_wifiState = WIFI_ST_CONNECTED;
                   break;

                   default:
                   break;
               }
            }
            break;

            case WIFI_ST_TCP_TRANSMITTING:
            {
                if (gs_appTask != NULL)
                {
                    vTaskResume(gs_appTask);
                }
            }
            break;

            case WIFI_ST_ERROR:
            default:
            {
                if (gs_appTask != NULL)
                {
                    vTaskResume(gs_appTask);
                }

                ge_wifiState = WIFI_ST_INITIALIZING;
            }
        }

        xSemaphoreGive(gs_wifiMutex);

        vTaskDelay(WIFI_TASK_DELAY);
    }

    vPortFree(gca_ssid);
    vPortFree(gca_pass);
    vPortFree(gca_ip);
    vPortFree(gca_addressToConnectTcp);
    vPortFree(gac_tcpHdrStr);
    vPortFree(gca_addr);
    vPortFree(gca_clientId);

    vTaskDelete(NULL);
}


/*****************************************************
 * Inicializa o módulo Wifi e a task de gerenciamento.
 *****************************************************/
bool WIFI_Init(void)
{
    gs_wifiMutex  = xSemaphoreCreateMutex();
    //TODO: Porque estoura? Pode ser memoria
    //gq_mqttMsgs   = xQueueCreate(5, sizeof(MqttMessage_st));
    //gq_mqttTopics = xQueueCreate(5, sizeof(MqttTopic_st));

    vQueueAddToRegistry(gs_wifiMutex, "wifi");

    xSemaphoreTake(gs_wifiMutex, portMAX_DELAY);    // Mutex.

    xTaskCreate( wifiTask                ,
                 (const char*) "Wifi"    ,
                 configMINIMAL_STACK_SIZE,
                 NULL                    ,
                 WIFI_TASK_PRIORITY      ,
                 NULL                     );

    // Aguarda inicialização:
    gs_appTask = xTaskGetCurrentTaskHandle();

    xSemaphoreGive(gs_wifiMutex);

    vTaskSuspend(gs_appTask);
    gs_appTask = NULL;

    return ge_wifiState == WIFI_ST_INITIALIZED;
}


/*****************************************************
 * Retorna o estado atual do gerenciamento do módulo.
 *****************************************************/
WifiState_et WIFI_GetState(void)
{
    return ge_wifiState;
}


/*****************************************************
 * Retorna o IP da conexão Wifi via parâmetro.
 *****************************************************/
bool WIFI_GetIp(char *ip)

{
    bool result = false;
    
    xSemaphoreTake(gs_wifiMutex, portMAX_DELAY);

    if ( (gca_ip != NULL) && strlen(gca_ip) )
    {
        strncpy(ip, gca_ip, WIFI_IP_MAX_LEN);
        result = true;
    }

    xSemaphoreGive(gs_wifiMutex);
    
    return result;
}


/*****************************************************
 * Desinicializa módulo.
 *****************************************************/
void WIFI_Deinit(void)
{
    AT_Deinit();

    //vQueueDelete(gq_mqttMsgs);
    //vQueueDelete(gq_mqttTopics);
}


/*****************************************************
 * Retorna o estado da conexão wifi (true se conectado
 * ou false se desconectado.
 *****************************************************/
bool WIFI_IsConnected(void)
{
    return (ge_wifiState >= WIFI_ST_CONNECTED) && (ge_wifiState < WIFI_ST_ERROR) ;
}


/*****************************************************
 * Retorna o estado da conexão wifi (true se conectado
 * ou false se desconectado.
 *****************************************************/
bool WIFI_Connect(const char* ssid, const char* pass)
{
    bool result = false;

    if ( ge_wifiState == WIFI_ST_INITIALIZED &&
         gca_ssid     != NULL                &&
         gca_pass     != NULL                  )
    {
        xSemaphoreTake(gs_wifiMutex, portMAX_DELAY);

        strncpy(gca_ssid, ssid, WIFI_SSID_MAX_LEN );
        strncpy(gca_pass, pass, WIFI_PASS_MAX_LEN );

        ge_wifiState = WIFI_ST_CONNECTING;

        gs_appTask = xTaskGetCurrentTaskHandle();

        xSemaphoreGive(gs_wifiMutex);

        vTaskSuspend(gs_appTask);
        gs_appTask = NULL;

        result = WIFI_IsConnected();
    }

    return result;
}


/*****************************************************
 * Desconecta da rede wifi.
 *****************************************************/
void WIFI_Disconnect(void)
{
    if ( WIFI_IsConnected() )
    {
        gca_ip[0]    = '\0';
        ge_wifiState = WIFI_ST_DISCONNECTING;
    }
}


/*****************************************************
 * Conecta em um servidor TCP. Recebe endereço e porta
 * e retorna o CID (connection ID) da conexão.
 *****************************************************/
uint8_t WIFI_TcpConnect (const char* address, uint16_t port)
{
    uint8_t result = 0xff;

    if ( WIFI_IsConnected() )
    {
        xSemaphoreTake(gs_wifiMutex, portMAX_DELAY);

        strncpy(gca_addressToConnectTcp, address, WIFI_SSID_MAX_LEN );
                gu16_portToConnectTcp  = port;

        ge_wifiState = WIFI_ST_TCP_CONNECTING;

        gs_appTask = xTaskGetCurrentTaskHandle();

        xSemaphoreGive(gs_wifiMutex);

        vTaskSuspend(gs_appTask);
        gs_appTask = NULL;

        result = gu8_currentCid;
    }

    return result;
}


/*****************************************************
 * Retorna a máscara dos CIDs (connection ID) das
 * conexões ativas.
 *****************************************************/
uint16_t  WIFI_TcpGetCids(void)
{
    return gu16_cidsMask;
}


/*****************************************************
 * Desconecta de uma conexão TCP.
 * Recebe o CID (connection ID) da conexão a ser
 * finalizada.
 *****************************************************/
bool WIFI_TcpDisconnect (uint8_t cid)
{
    bool result = true;

    return result;
}


/*****************************************************
 * Envia dados via TCP para o host remoto.
 * Envia o CID (connection ID) da conexão, buffer de
 * dados e tamanho.
 *****************************************************/
bool WIFI_TcpWrite(uint8_t cid, uint8_t* data, uint16_t dataLen)
{
    bool result = false;

    xSemaphoreTake(gs_wifiMutex, portMAX_DELAY);

    if ( (ge_wifiState == WIFI_ST_TCP_CONNECTED) &&  (WIFI_TcpGetCids() & (0x0001 << cid)) )
    {
        char* bulkHeader = pvPortMalloc(16+1);

        sprintf(bulkHeader,"\x1bZ%1x%04d", cid, dataLen);

        AT_Write(SERIAL_PORT2, bulkHeader, strlen(bulkHeader));
        AT_Write(SERIAL_PORT2, data      , dataLen           );

        vPortFree(bulkHeader);

        result = true;
    }

    xSemaphoreGive(gs_wifiMutex);

    return result;  
}

#ifdef NATIVE_MQTT
/**
 * MQTT Functions
 */

WifiState_et MQTT_GetState(void)
{
    return ge_wifiState;
}

bool MQTT_IsConnected(void)
{
    (ge_wifiState >= MQTT_ST_CONNECTED) && (ge_wifiState < MQTT_ST_ERROR);
}

uint8_t MQTT_Connect(const char* address, uint16_t port, const char* clientId, const char* usr, const char* pwd)
{
    bool result = false;

    if ( WIFI_IsConnected()  &&
         address     != NULL                 &&
         clientId    != NULL                 &&
         port        != 0                     )
    {
        xSemaphoreTake(gs_wifiMutex, portMAX_DELAY);

        strncpy(gca_addr,     address,  MQTT_ADDRESS_MAX_LEN);
        strncpy(gca_clientId, clientId, MQTT_CLIENT_ID_MAX_LEN);

        gu16_port = port;

        ge_wifiState = MQTT_ST_CONNECTING;

        gs_appTask = xTaskGetCurrentTaskHandle();

        xSemaphoreGive(gs_wifiMutex);

        vTaskSuspend(gs_appTask);
        gs_appTask = NULL;

        result = MQTT_IsConnected();
    }

    return result;
}

bool MQTT_Publish(uint8_t cid, const char* topic, uint32_t len, uint8_t *data, uint8_t qos, bool retain)
{
    MqttMessage_st msg = {0};

    msg.len    = len;
    msg.cid    = cid;
    msg.qos    = qos;
    msg.retain = retain;

    strncpy(&msg.topic,   topic,   MQTT_TOPIC_MAX_LEN);
    memcpy(&msg.data, data, len);

    xQueueSend(gq_mqttMsgs, (MqttMessage_st*)&msg, NULL);

    ge_wifiState = MQTT_ST_PUBLISHING;
}

bool MQTT_Subscribe(uint8_t cid, const char* topic, uint8_t qos, uint16_t timeout)
{
    MqttTopic_st mqttTopic = {0};

    mqttTopic.cid     = cid;
    mqttTopic.qos     = qos;
    mqttTopic.timeout = timeout;

    strncpy(&mqttTopic.topic,   topic,   MQTT_TOPIC_MAX_LEN);

    xQueueSend(gq_mqttTopics, (MqttTopic_st*)&topic, NULL);

    ge_wifiState = MQTT_ST_SUBSCRIBING;
}

bool MQTT_Ping(uint8_t cid, uint16_t timeout)
{
    //TODO
}

void MQTT_Disconnect(uint8_t cid)
{
    if ( MQTT_IsConnected() )
    {
        gu8_mqttCurrentCid = cid;
        ge_wifiState = MQTT_ST_DISCONNECTING;
    }
}

#endif //NATIVE_MQTT

bool MQTT_Write(uint8_t cid, uint8_t* data, uint16_t dataLen)
{
    bool result = false;

    xSemaphoreTake(gs_wifiMutex, portMAX_DELAY);

    if ( (ge_wifiState == MQTT_ST_CONNECTED) /*TODO: && (MQTT_GetCids() & (0x0001 << cid))*/ )
    {
        char* header = pvPortMalloc(16+1);

        sprintf(header,"\x1bN%1x", cid);

        AT_Write(SERIAL_PORT2, header, strlen(header));
        AT_Write(SERIAL_PORT2, data  , dataLen       );

        vPortFree(header);

        result = true;
    }

    xSemaphoreGive(gs_wifiMutex);

    return result;  
}


/*****************************************************
 * Trata a recepção dos dados TCP.
 *****************************************************/

bool WIFI_HandleReceivedData(uint8_t data)
{
    bool result = false;

    // Decodifica formato bulk:
    if (!gb_tcpHdrReceived)
    {
        // Verifica se recebeu um "\x1bZ<cid:1><len ascii:4>":
        if (gu8_tcpHdrIndex < TCP_HEADER_ID_LEN)
        {
            if ( data == gac_tcpHdrStr[gu8_tcpHdrIndex] )
            {
                gu8_tcpHdrIndex++;
            }
            else
            {
                gu8_tcpHdrIndex = data == gac_tcpHdrStr[0] ? 1 : 0;
            }
        } // Recebe CID:
        else if (gu8_tcpHdrIndex == TCP_HEADER_ID_LEN)
        {
            if ( (gac_tcpHdrStr[gu8_tcpHdrIndex] = charToByteHex(data)) <= 0x0f )
            {
                gu8_tcpHdrIndex++;
                gu8_currentCid = gac_tcpHdrStr[TCP_HEADER_ID_LEN];
            }
            else
            {
                gu8_tcpHdrIndex = data == gac_tcpHdrStr[0] ? 1 : 0;
            }
        } // Recebe tamanho:
        else if (gu8_tcpHdrIndex < TCP_HEADER_LEN)
        {
            if (data >= '0' && data <= '9')
            {
                gac_tcpHdrStr[gu8_tcpHdrIndex++] = data;

                if (gu8_tcpHdrIndex == TCP_HEADER_LEN)
                {
                    sscanf(gac_tcpHdrStr+3,"%4d", &gu32_tcpDataLen);

                    gu8_tcpHdrIndex   = 0;
                    gu8_tcpDataIndex  = 0;
                    gb_tcpHdrReceived = true;
                }
            }
            else
            {
                gu8_tcpHdrIndex = data == gac_tcpHdrStr[0] ? 1 : 0;
            }
        } // Processa cabeçalho:
    } // Recebe dados TCP.
    else
    {
        if (gu8_tcpDataIndex < gu32_tcpDataLen)
        {
            gu8_tcpDataIndex++;

            result = true;

            if (gu8_tcpDataIndex == gu32_tcpDataLen)
            {
                gb_tcpHdrReceived = false;
            }                
        }
    }

    return result;   
}

#ifndef AT_MULTI_SERIAL
void AT_ReadCallback(uint8_t data, SerialPort_et port)
{
    if (WIFI_HandleReceivedData(data))
    {
        WIFI_TcpReadCallback(data, port);
    }
}
#endif