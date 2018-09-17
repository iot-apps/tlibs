#include <asf.h>
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#include "serial.h"

/******************************************************************************
 *                                 DECLARAÇÕES
 ******************************************************************************/

// Variáveis
static struct usart_module gsUsartSerialModule;
static uint16_t            gu16RecvData;
static SemaphoreHandle_t   serialMutex      = NULL;

//Protótipos
static void receiveCallback(struct usart_module *const module);

/******************************************************************************
 *                             SERIAL
 ******************************************************************************/

SerialResult_et SERIAL_Init(void)
{
    serialMutex = xSemaphoreCreateMutex();

    vQueueAddToRegistry(serialMutex, "serialMutex");

    struct usart_config usart_conf;
    
    usart_get_config_defaults(&usart_conf);
    
    usart_conf.baudrate       = SERIAL_BAUDRATE;
    usart_conf.character_size = SERIAL_DATABIT;
    usart_conf.stopbits       = SERIAL_STOP_BITS;
    usart_conf.parity         = SERIAL_PARITY;
    
    usart_conf.mux_setting    = SERIAL_MUX_SETTINGS;
    usart_conf.pinmux_pad0    = SERIAL_PAD0;
    usart_conf.pinmux_pad1    = SERIAL_PAD1;
    usart_conf.pinmux_pad2    = SERIAL_PAD2;
    usart_conf.pinmux_pad3    = SERIAL_PAD3;
    
    while (usart_init(&gsUsartSerialModule, SERIAL_SERCOM, &usart_conf) != STATUS_OK);
    
    usart_enable(&gsUsartSerialModule);
      
    usart_register_callback(&gsUsartSerialModule, receiveCallback, USART_CALLBACK_BUFFER_RECEIVED);
    usart_enable_callback(&gsUsartSerialModule, USART_CALLBACK_BUFFER_RECEIVED);
                 
    while (usart_read_job(&gsUsartSerialModule, &gu16RecvData) != STATUS_OK);

    return SERIAL_RESULT_OK;
}

SerialResult_et SERIAL_Deinit(void)
{
    xSemaphoreTake(serialMutex, portMAX_DELAY);
    usart_disable_callback(&gsUsartSerialModule, USART_CALLBACK_BUFFER_RECEIVED);
    usart_disable(&gsUsartSerialModule);
    xSemaphoreGive(serialMutex);

    return SERIAL_RESULT_OK;
}

SerialResult_et SERIAL_Write(const void *data, uint16_t length)
{
    xSemaphoreTake(serialMutex, portMAX_DELAY);
    assert(data != NULL);
    while (usart_write_buffer_wait(&gsUsartSerialModule, (uint8_t *) data, length) != STATUS_OK);
    xSemaphoreGive(serialMutex);

    return SERIAL_RESULT_OK;
}

// SerialResult_et SERIAL_Print(const char *data)
// {
//     uint16_t i = 0;
//     
//     while (data[i] != '\0')
//         while(usart_write_wait(&gsUsartSerialModule, data[i++]) != STATUS_OK);
// 
//       
//     return SERIAL_RESULT_OK;
// }

void SERIAL_Print(const char *data)
{
    xSemaphoreTake(serialMutex, portMAX_DELAY);

    while (*data)
    {
        while(usart_write_wait(&gsUsartSerialModule, *data) != STATUS_OK);
        data++;
    }

    xSemaphoreGive(serialMutex);
}


void SERIAL_Println(const char *data)
{
    SERIAL_Print(data);
    SERIAL_Print("\r\n");
}


/******************************************************************************
 *
 *                             FUNÇÕES ESTÁTICAS
 *
******************************************************************************/

static void receiveCallback(struct usart_module *const module)
{
    while (usart_read_job(&gsUsartSerialModule, &gu16RecvData) != STATUS_OK);

    // Implementado pelo usuário.
    SERIAL_ReadCallback((uint8_t) gu16RecvData);    
}
