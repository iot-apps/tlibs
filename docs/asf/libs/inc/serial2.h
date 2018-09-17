/*
 * serial2.h
 *
 * Created: 13/03/2018 17:00:51
 *  Author: Jonas
 */ 


#ifndef SERIAL2_H_
#define SERIAL2_H_

#include <asf.h>

#include <stdbool.h>
#include <stdint.h>


typedef enum
{
    SERIAL2_RESULT_OK,
    SERIAL2_RESULT_UART_ERROR,
    SERIAL2_RESULT_INVALID_ARG,
    SERIAL2_RESULT_NOT_FOUND
} Serial2Result_et;


Serial2Result_et SERIAL2_Init  (void);
Serial2Result_et SERIAL2_Deinit(void);
Serial2Result_et SERIAL2_Write (const void *data, uint16_t length);
void SERIAL2_Print(const char *data);
void SERIAL2_Println(const char *data);

// Implementado pelo usuário
void SERIAL2_ReadCallback(uint8_t data);



#endif /* SERIAL2_H_ */