/*
 * uartSerial.h
 *
 * Created: 13/03/2018 16:58:49
 *  Author: Jonas
 */ 


#ifndef UARTSERIAL_H_
#define UARTSERIAL_H_


#include <stdint.h>


typedef enum
{
    SERIAL_RESULT_OK,
    SERIAL_RESULT_UART_ERROR,
    SERIAL_RESULT_INVALID_ARG,
    SERIAL_RESULT_NOT_FOUND
} SerialResult_et;


SerialResult_et SERIAL_Init  (void);
SerialResult_et SERIAL_Deinit(void);
SerialResult_et SERIAL_Write (const void *data, uint16_t length);
void SERIAL_Print(const char *data);
void SERIAL_Println(const char *data);

// Implementado pelo usuário
void SERIAL_ReadCallback(uint8_t data);



#endif /* UARTSERIAL_H_ */