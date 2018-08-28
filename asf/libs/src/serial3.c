#include <asf.h>
#include <assert.h>

#include "serial3.h"

/******************************************************************************
 *                                 DECLARAÇÕES
 ******************************************************************************/

// Variáveis
static struct usart_module gsUsartSerial3Module;
static uint16_t            gu16RecvData;

// Protótipos
static        void receiveCallback3(struct usart_module *const module);
static inline void setResetPin    (bool reset);


/******************************************************************************
 *                             INTERFACE SERIAL AUXILIAR
 ******************************************************************************/

// Inicializa Serial 3.
Serial3Result_et SERIAL3_Init(void)
{
    struct usart_config usart_conf;
    
    usart_get_config_defaults(&usart_conf);
    
    usart_conf.baudrate       = SERIAL3_BAUDRATE;
    usart_conf.character_size = SERIAL3_DATABIT;
    usart_conf.parity         = SERIAL3_PARITY;
    usart_conf.stopbits       = SERIAL3_STOP_BITS;
    
    usart_conf.mux_setting    = SERIAL3_MUX_SETTINGS;
    usart_conf.pinmux_pad0    = SERIAL3_PAD0;
    usart_conf.pinmux_pad1    = SERIAL3_PAD1;
    usart_conf.pinmux_pad2    = SERIAL3_PAD2;
    usart_conf.pinmux_pad3    = SERIAL3_PAD3;
    
    while (usart_init(&gsUsartSerial3Module, SERIAL3_SERCOM, &usart_conf) != STATUS_OK);
    usart_enable(&gsUsartSerial3Module);
    
    usart_register_callback(&gsUsartSerial3Module, receiveCallback3, USART_CALLBACK_BUFFER_RECEIVED);
    usart_enable_callback(&gsUsartSerial3Module, USART_CALLBACK_BUFFER_RECEIVED);
    
    while (usart_read_job(&gsUsartSerial3Module, &gu16RecvData) != STATUS_OK);
    
    return SERIAL3_RESULT_OK;
}

// Desconfigura e desliga o módulo Wi-Fi.
Serial3Result_et SERIAL3_Deinit(void)
{
    usart_disable_callback(&gsUsartSerial3Module, USART_CALLBACK_BUFFER_RECEIVED);
    usart_disable(&gsUsartSerial3Module);
    
    return SERIAL3_RESULT_OK;
}

// Escreve na serial referente ao módulo.
Serial3Result_et SERIAL3_Write(const void *data, uint16_t length)
{
    assert(data != NULL);

    while (usart_write_buffer_wait(&gsUsartSerial3Module, (uint8_t *) data, length) != STATUS_OK);
    return SERIAL3_RESULT_OK;
}

void SERIAL3_Print(const char *data)
{
    while (*data)
    {
        while(usart_write_wait(&gsUsartSerial3Module, *data) != STATUS_OK);
        data++;
    }
}


void SERIAL3_Println(const char *data)
{
    SERIAL3_Print(data);
    SERIAL3_Print("\r\n");
}

/******************************************************************************
 *                             FUNÇÕES ESTÁTICAS
 ******************************************************************************/

// Recebe bytes do módulo pela serial.
static void receiveCallback3(struct usart_module *const module)
{
    while (usart_read_job(&gsUsartSerial3Module, &gu16RecvData) != STATUS_OK);

    SERIAL3_ReadCallback((uint8_t) gu16RecvData);
}
