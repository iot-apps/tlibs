/*
 * at.c
 *
 * Created: 15/03/2018 16:14:34
 *  Author: Jonas
 */ 

#include <asf.h>
#include <stdio.h>
#include <string.h>

#include "at.h"


#define AT_HEADER_LEN               2   // "\r\n"

/******************************************************************************
 *                               PROTÓTIPOS LOCAIS
 ******************************************************************************/
static void callbackHandler(uint8_t data);
static void callbackHandler2(uint8_t data);

static const char       gac_newLineStr[]       = "\r\n";
static uint8_t          gu8_newLineIndex       = 0;
static bool             gb_newLineReceived     = false;
static uint8_t          gu8_newLineIndexSer3   = 0;
static bool             gb_newLineReceivedSer3 = false;

static const char       gac_okStr[]       = "\r\nOK\r\n";
static uint8_t          gu8_okIndex       = 0;
static bool             gb_okReceived     = false;
static uint8_t          gu8_okIndexSer3   = 0;
static bool             gb_okReceivedSer3 = false;

static const char       gac_errorStr[]       = "\r\nERROR\r\n";
static uint8_t          gu8_errorIndex       = 0;
static bool             gb_errorReceived     = false;
static uint8_t          gu8_errorIndexSer3   = 0;
static bool             gb_errorReceivedSer3 = false;

static AtState_et       ge_requestState    = AT_ST_SND;
static QueueHandle_t    dataReceivedQueue  = NULL;
static QueueHandle_t    dataReceivedQueue2 = NULL;
static char**           gapc_atCommands    = NULL;


void AT_Init(const char** atCommands)
{
    gapc_atCommands    = atCommands;
    dataReceivedQueue  = xQueueCreate(200, sizeof(uint8_t));
    dataReceivedQueue2 = xQueueCreate(200, sizeof(uint8_t));

    SERIAL2_Init();

    SERIAL2_Println("AT\r\n");

    SERIAL3_Init();
    
    SERIAL3_Println("AT\r\n");    
}


void AT_Deinit(void)
{
    SERIAL2_Deinit();

    SERIAL3_Deinit();

    vQueueDelete(dataReceivedQueue);
    vQueueDelete(dataReceivedQueue2);
}

AtState_et AT_Request(uint8_t commandIndex, char* parameters, char* receivedParams, uint16_t receivedParamsMaxLen, uint16_t timeout)
{
    return AT_RequestSerial(SERIAL_PORT2, commandIndex, parameters, receivedParams, receivedParamsMaxLen, timeout);
}

AtState_et AT_RequestSerial(SerialPort_et port, uint8_t commandIndex, char* parameters, char* receivedParams, uint16_t receivedParamsMaxLen, uint16_t timeout)
{
    if ( ge_requestState != AT_ST_RCV_WAITING )
    {
        ge_requestState = AT_ST_SND;
    }

    switch(ge_requestState)
    {
        case AT_ST_SND:
        {
            if ( AT_SendCommand(port, commandIndex, parameters) == AT_ST_SND_OK )
            {
                ge_requestState = AT_ST_RCV_WAITING;
            }
            else
            {
                ge_requestState = AT_ST_ERROR;
            }
        }
        break;

        case AT_ST_RCV_WAITING:
        {
            uint16_t receivedParamsLen = 0;

            ge_requestState = AT_CommandReceived(port, (void*)receivedParams, receivedParamsMaxLen, &receivedParamsLen, timeout);
        }
        break;

        default:
        {
            ge_requestState = AT_ST_ERROR;
        }
        break;
    }

    return ge_requestState;
}


AtState_et AT_SendCommand(SerialPort_et port, uint8_t commandIndex, char* parameters)
{
    AtState_et result                                    = AT_ST_ERROR;
    char       commandToSend[AT_MAX_COMMAND_STRING_SIZE+1] = {0};
	uint16_t   commandSize = strlen(gapc_atCommands[commandIndex]);

    //if ( commandIndex < sizeof(gapc_atCommands) )
    {
        if ( (parameters == NULL) || (strlen(parameters) == 0) )
        {
            strncpy( commandToSend, gapc_atCommands[commandIndex], AT_MAX_COMMAND_STRING_SIZE );
        }
        else if ( (commandSize + 1 + strlen(parameters) ) < AT_MAX_COMMAND_STRING_SIZE )
        {
            strncat(commandToSend, gapc_atCommands[commandIndex], AT_MAX_COMMAND_STRING_SIZE);
			if (commandToSend[commandSize-1] != 0x20) //TODO: Melhorar solução para quando o comando com parametros não tiver o "="
				strncat(commandToSend, "="                      , AT_MAX_COMMAND_STRING_SIZE);
            strncat(commandToSend, parameters                   , AT_MAX_COMMAND_STRING_SIZE);
        }
    }

    if ( strlen(commandToSend) > 0)
    {
        xQueueReset(dataReceivedQueue);
        xQueueReset(dataReceivedQueue2);

        AT_Println(port, commandToSend);

        result = AT_ST_SND_OK;
    }

    return result;
}


AtState_et AT_CommandReceived(SerialPort_et port, void* receivedParams, uint16_t receivedParamsMaxLen, uint16_t *receivedParamsLen, uint16_t timeout)
{
    AtState_et result = AT_ST_RCV_WAITING;
    uint8_t*   buffer = (uint8_t*)receivedParams;

    if (gb_errorReceived)
    {
        gb_errorReceived = false;
        result           = AT_ST_ERROR;
    }
    else if (gb_okReceived)
    {
        gb_okReceived = false;
        result        = AT_ST_RCV_OK;
    }

    if (buffer != NULL)
    {
        uint16_t len = 0;
        char     ch  = 0;

        if (result != AT_ST_ERROR)
        {
            switch(port)
            {
                case SERIAL_PORT2:
                {
                    while ( xQueueReceive(dataReceivedQueue, &ch, 0) && (len < receivedParamsMaxLen) )
                    {
                        buffer[len++] = ch;
                    }
                }
                break;
                case SERIAL_PORT3:
                {
                    while ( xQueueReceive(dataReceivedQueue2, &ch, 0) && (len < receivedParamsMaxLen) )
                    {
                        buffer[len++] = ch;
                    }
                }
                break;
                default: break;
            }

            buffer[len] = '\0';

            if (len > 0)
            {
                result = AT_ST_RCV_OK_PARAM;
            }
       }
    }

    return result;
}


void AT_Write(SerialPort_et port, void* data, uint16_t dataLen)
{
    switch(port)
    {
        case SERIAL_PORT2:
        {
            SERIAL2_Write(data, dataLen);
        }
        break;
        case SERIAL_PORT3:
        {
            SERIAL3_Write(data, dataLen);
        }
        break;
        default: break;
    }
}

void AT_Println(SerialPort_et port, char* data)
{
    switch(port)
    {
        case SERIAL_PORT2:
        {
            SERIAL2_Println(data);
        }
        break;
        case SERIAL_PORT3:
        {
            SERIAL3_Println(data);
        }
        break;
        default: break;
    }
}

void SERIAL3_ReadCallback (uint8_t data)
{
    callbackHandler2(data);

    xQueueSendFromISR(dataReceivedQueue2, (uint8_t*)&data, NULL);

    AT_ReadCallback(data, SERIAL_PORT3);
}

void SERIAL2_ReadCallback(uint8_t data)
{
    callbackHandler(data);
    xQueueSendFromISR(dataReceivedQueue, (uint8_t*)&data, NULL);

    AT_ReadCallback(data, SERIAL_PORT2);
}

static void callbackHandler2(uint8_t data)
{
    //    SERIAL_Write(&data, 1);

   // If receive a newline character:
    if (data == gac_newLineStr[gu8_newLineIndexSer3++])
    {
        if (gu8_newLineIndexSer3 == (sizeof(gac_newLineStr)-1) )
        {
            gu8_newLineIndexSer3   = 0;
            gb_newLineReceivedSer3 = true;
        }
    }
    else
    {
        gu8_newLineIndexSer3 = data == gac_newLineStr[0] ? 1 : 0;
    }

    // Verifica se recebeu um OK:
    if ( data == gac_okStr[gu8_okIndexSer3++] )
    {
        if ( gu8_okIndexSer3 == (sizeof(gac_okStr)-1) )
        {
            gu8_okIndexSer3   = 0;
            gb_okReceivedSer3 = true;
        }
    }
    else
    {
        gu8_okIndexSer3 = data == gac_okStr[0] ? 1 : 0;
    }

    // Verifica se recebeu um ERROR:
    if (data == gac_errorStr[gu8_errorIndexSer3++])
    {
        if ( gu8_errorIndexSer3 == (sizeof(gac_errorStr)-1) )
        {
            gu8_errorIndexSer3   = 0;
            gb_errorReceivedSer3 = true;
        }
    }
    else
    {
        gu8_errorIndexSer3 = data == gac_errorStr[0] ? 1 : 0;
    }
}

static void callbackHandler(uint8_t data)
{
    //    SERIAL_Write(&data, 1);

   // If receive a newline character:
    if (data == gac_newLineStr[gu8_newLineIndex++])
    {
        if (gu8_newLineIndex == (sizeof(gac_newLineStr)-1) )
        {
            gu8_newLineIndex   = 0;
            gb_newLineReceived = true;
        }
    }
    else
    {
        gu8_newLineIndex = data == gac_newLineStr[0] ? 1 : 0;
    }

    // Verifica se recebeu um OK:
    if ( data == gac_okStr[gu8_okIndex++] )
    {
        if ( gu8_okIndex == (sizeof(gac_okStr)-1) )
        {
            gu8_okIndex   = 0;
            gb_okReceived = true;
        }
    }
    else
    {
        gu8_okIndex = data == gac_okStr[0] ? 1 : 0;
    }

    // Verifica se recebeu um ERROR:
    if (data == gac_errorStr[gu8_errorIndex++])
    {
        if ( gu8_errorIndex == (sizeof(gac_errorStr)-1) )
        {
            gu8_errorIndex   = 0;
            gb_errorReceived = true;
        }
    }
    else
    {
        gu8_errorIndex = data == gac_errorStr[0] ? 1 : 0;
    }
}
