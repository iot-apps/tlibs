#include <asf.h>
#include <assert.h>

#include "serial2.h"
#include "serial3.h"

/******************************************************************************
 *                                 DECLARAÇÕES
 ******************************************************************************/

// Variáveis
static struct usart_module gsUsartSerial2Module;
static uint16_t            gu16RecvData;

// Protótipos
static        void receiveCallback(struct usart_module *const module);
static inline void setResetPin    (bool reset);


/******************************************************************************
 *                             INTERFACE SERIAL AUXILIAR
 ******************************************************************************/

// Inicializa Serial 2.
Serial2Result_et SERIAL2_Init(void)
{
    struct usart_config usart_conf;
    
    usart_get_config_defaults(&usart_conf);
    
    usart_conf.baudrate       = SERIAL2_BAUDRATE;
    usart_conf.character_size = SERIAL2_DATABIT;
    usart_conf.parity         = SERIAL2_PARITY;
    usart_conf.stopbits       = SERIAL2_STOP_BITS;
    
    usart_conf.mux_setting    = SERIAL2_MUX_SETTINGS;
    usart_conf.pinmux_pad0    = SERIAL2_PAD0;
    usart_conf.pinmux_pad1    = SERIAL2_PAD1;
    usart_conf.pinmux_pad2    = SERIAL2_PAD2;
    usart_conf.pinmux_pad3    = SERIAL2_PAD3;
    
    while (usart_init(&gsUsartSerial2Module, SERIAL2_SERCOM, &usart_conf) != STATUS_OK);
    usart_enable(&gsUsartSerial2Module);
    
    usart_register_callback(&gsUsartSerial2Module, receiveCallback, USART_CALLBACK_BUFFER_RECEIVED);
    usart_enable_callback(&gsUsartSerial2Module, USART_CALLBACK_BUFFER_RECEIVED);
    
    while (usart_read_job(&gsUsartSerial2Module, &gu16RecvData) != STATUS_OK);
    
    return SERIAL2_RESULT_OK;
}

// Desconfigura e desliga o módulo Wi-Fi.
Serial2Result_et SERIAL2_Deinit(void)
{
    usart_disable_callback(&gsUsartSerial2Module, USART_CALLBACK_BUFFER_RECEIVED);
    usart_disable(&gsUsartSerial2Module);
    
    return SERIAL2_RESULT_OK;
}

// Escreve na serial referente ao módulo.
Serial2Result_et SERIAL2_Write(const void *data, uint16_t length)
{
    assert(data != NULL);

    while (usart_write_buffer_wait(&gsUsartSerial2Module, (uint8_t *) data, length) != STATUS_OK);
    return SERIAL2_RESULT_OK;
}

void SERIAL2_Print(const char *data)
{
    while (*data)
    {
        while(usart_write_wait(&gsUsartSerial2Module, *data) != STATUS_OK);
        data++;
    }
}


void SERIAL2_Println(const char *data)
{
    SERIAL2_Print(data);
    SERIAL2_Print("\r\n");
}

/******************************************************************************
 *                             FUNÇÕES ESTÁTICAS
 ******************************************************************************/

// Recebe bytes do módulo pela serial.
static void receiveCallback(struct usart_module *const module)
{
    while (usart_read_job(&gsUsartSerial2Module, &gu16RecvData) != STATUS_OK);

    SERIAL2_ReadCallback((uint8_t) gu16RecvData);
}
