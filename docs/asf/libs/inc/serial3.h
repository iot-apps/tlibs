/*
 * serial3.h
 *
 * Created: 18/07/2018 13:01:32
 *  Author: Jonas
 */ 


#ifndef SERIAL3_H_
#define SERIAL3_H_

#include <asf.h>

#include <stdbool.h>
#include <stdint.h>


typedef enum
{
    SERIAL3_RESULT_OK,
    SERIAL3_RESULT_UART_ERROR,
    SERIAL3_RESULT_INVALID_ARG,
    SERIAL3_RESULT_NOT_FOUND
} Serial3Result_et;


Serial3Result_et SERIAL3_Init  (void);
Serial3Result_et SERIAL3_Deinit(void);
Serial3Result_et SERIAL3_Write (const void *data, uint16_t length);
void SERIAL3_Print(const char *data);
void SERIAL3_Println(const char *data);

// Implementado pelo usuário
void SERIAL3_ReadCallback(uint8_t data);



#endif /* SERIAL3_H_ */